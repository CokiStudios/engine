#pragma once

#include "LoopingValue.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <atomic>
#include <cmath>
#include <cctype>
#include <cstring>
#include <limits>
#include <algorithm>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define popen _popen
#define pclose _pclose
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace looping {

class PythonBridge {
public:
    std::string detected_exe;
    std::vector<std::string> detected_args;
    bool is_available = false;
    bool has_probed = false;

    PythonBridge() = default;

    bool ensure_detected() {
        if (!has_probed) {
            detect();
        }
        return is_available;
    }

    bool check_available() {
        return ensure_detected();
    }

    void detect() {
        has_probed = true;
#ifdef __EMSCRIPTEN__
        detected_exe = "";
        detected_args.clear();
        is_available = false;
        return;
#else
        std::vector<std::pair<std::string, std::vector<std::string>>> candidates;
#if defined(_WIN32)
        candidates.push_back({"python", {}});
        candidates.push_back({"py", {"-3"}});
        candidates.push_back({"py", {}});
        candidates.push_back({"python3", {}});
#else
        candidates.push_back({"python3", {}});
        candidates.push_back({"python", {}});
        candidates.push_back({"py", {}});
#endif

        for (const auto& [exe, args] : candidates) {
            std::string cmd = exe;
            for (const auto& a : args) cmd += " " + a;
            cmd += " -c \"import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')\"";

            int status = -1;
            std::string result = run_captured_command(cmd, &status);

            // Trim
            while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) {
                result.pop_back();
            }

            if (status == 0 && (result.rfind("3.", 0) == 0 || result.rfind("2.7", 0) == 0)) {
                detected_exe = exe;
                detected_args = args;
                is_available = true;
                return;
            }
        }
        is_available = false;
#endif
    }

    std::string get_command_str() {
        if (!ensure_detected()) return "";
        std::string s = detected_exe;
        for (const auto& a : detected_args) s += " " + a;
        return s;
    }

    // Native C++17 fast-path for pure Python math/expression evaluation (zero Python dependency)
    static bool try_native_eval(const std::string& expr,
                                const std::unordered_map<std::string, Value>& variables,
                                Value& out_val) {
        NativePyParser parser(expr, variables);
        if (!parser.parse_full(out_val)) {
            return false;
        }
        return true;
    }

    // Execute multiline Python script with bidirectional variable sync
    std::string execute_block(const std::string& py_code, std::unordered_map<std::string, Value>& variables) {
        if (!ensure_detected()) {
            return "[Error: Python executable not found in PATH]";
        }

        // Build variables JSON
        std::string vars_json = "{";
        bool first = true;
        for (const auto& [k, v] : variables) {
            if (!first) vars_json += ", ";
            first = false;
            vars_json += "\"" + k + "\": " + v.to_json();
        }
        vars_json += "}";

        std::string temp_py_path = next_temp_py_path("looping_py_block_");
        FILE* f = fopen(temp_py_path.c_str(), "w");
        if (!f) return "[Error: Cannot create temporary script]";

        std::string bootstrap =
            "import sys, json, os, math\n"
            "try:\n"
            "    looping_vars = json.loads(os.environ.get('LOOPING_VARS_JSON', '{}'))\n"
            "    for _k, _v in looping_vars.items():\n"
            "        globals()[_k] = _v\n"
            "except Exception:\n"
            "    looping_vars = {}\n"
            "\n"
            "def looping_set(key, val):\n"
            "    looping_vars[key] = val\n"
            "    globals()[key] = val\n"
            "    sys.stdout.write(f'__LOOPING_SET__:' + json.dumps({'k': str(key), 'v': val}) + '\\n')\n"
            "    sys.stdout.flush()\n"
            "\n" + py_code + "\n";

        fputs(bootstrap.c_str(), f);
        fclose(f);

#if defined(_WIN32)
        SetEnvironmentVariableA("LOOPING_VARS_JSON", vars_json.c_str());
#else
        setenv("LOOPING_VARS_JSON", vars_json.c_str(), 1);
#endif

        int status = -1;
        std::string cmd = get_command_str() + " \"" + temp_py_path + "\"";
        std::string raw_output = run_captured_command(cmd, &status);
        remove(temp_py_path.c_str());

        if (status == -127) {
            return "[Error: Failed to execute python]";
        }

        std::string stdout_clean = "";
        std::istringstream iss(raw_output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            const std::string prefix = "__LOOPING_SET__:";
            auto pos = line.find(prefix);
            if (pos != std::string::npos) {
                std::string payload = line.substr(pos + prefix.length());
                parse_and_set_var(payload, variables);
            } else {
                if (!stdout_clean.empty()) stdout_clean += "\n";
                stdout_clean += line;
            }
        }

        while (!stdout_clean.empty() && (stdout_clean.back() == '\n' || stdout_clean.back() == '\r')) {
            stdout_clean.pop_back();
        }

        return stdout_clean;
    }

    // Evaluate inline Python expression (uses native C++ fast-path first to avoid Python dependency)
    Value eval_expr(const std::string& expr, const std::unordered_map<std::string, Value>& variables) {
        Value native_res;
        if (try_native_eval(expr, variables, native_res)) {
            return native_res;
        }

        if (!ensure_detected()) return Value();

        std::string vars_json = "{";
        bool first = true;
        for (const auto& [k, v] : variables) {
            if (!first) vars_json += ", ";
            first = false;
            vars_json += "\"" + k + "\": " + v.to_json();
        }
        vars_json += "}";

#if defined(_WIN32)
        SetEnvironmentVariableA("LOOPING_VARS_JSON", vars_json.c_str());
#else
        setenv("LOOPING_VARS_JSON", vars_json.c_str(), 1);
#endif

        std::string py_script =
            "import sys, json, os, math\n"
            "v = json.loads(os.environ.get('LOOPING_VARS_JSON', '{}'))\n"
            "globals().update(v)\n"
            "try:\n"
            "    res = (" + expr + ")\n"
            "    print(json.dumps(res))\n"
            "except Exception:\n"
            "    sys.exit(1)\n";

        std::string temp_py_path = next_temp_py_path("looping_eval_");
        FILE* f = fopen(temp_py_path.c_str(), "w");
        if (!f) return Value();
        fputs(py_script.c_str(), f);
        fclose(f);

        int status = -1;
        std::string cmd = get_command_str() + " \"" + temp_py_path + "\"";
        std::string output = run_captured_command(cmd, &status);
        remove(temp_py_path.c_str());

        while (!output.empty() && (output.back() == '\n' || output.back() == '\r' || output.back() == ' ')) {
            output.pop_back();
        }

        if (status == 0 && !output.empty()) {
            return parse_json_value(output);
        }

        return Value();
    }

private:
    static std::string next_temp_py_path(const std::string& prefix) {
        static std::atomic<uint64_t> counter{0};
        uint64_t seq = ++counter;
#if defined(_WIN32)
        char tmp_dir[MAX_PATH];
        GetTempPathA(MAX_PATH, tmp_dir);
        return std::string(tmp_dir) + prefix + std::to_string(GetCurrentProcessId()) + "_" + std::to_string(seq) + ".py";
#else
        return "/tmp/" + prefix + std::to_string(getpid()) + "_" + std::to_string(seq) + ".py";
#endif
    }

    static std::string run_captured_command(const std::string& cmd, int* out_status) {
#ifdef __EMSCRIPTEN__
        (void)cmd;
        if (out_status) *out_status = -127;
        return "";
#elif defined(_WIN32)
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        HANDLE hRead = NULL, hWrite = NULL;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            if (out_status) *out_status = -127;
            return "";
        }
        SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si{};
        si.cb = sizeof(STARTUPINFOA);
        si.hStdOutput = hWrite;
        si.hStdError = hWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.dwFlags |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION pi{};
        std::vector<char> cmd_buf(cmd.begin(), cmd.end());
        cmd_buf.push_back('\0');

        BOOL ok = CreateProcessA(
            NULL, cmd_buf.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi
        );
        CloseHandle(hWrite);

        if (!ok) {
            CloseHandle(hRead);
            if (out_status) *out_status = -127;
            return "";
        }

        std::string output;
        char buffer[1024];
        DWORD bytesRead = 0;
        while (ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
            output.append(buffer, static_cast<size_t>(bytesRead));
        }
        CloseHandle(hRead);
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (out_status) *out_status = static_cast<int>(exitCode);
        return output;
#else
        std::string full_cmd = cmd + " 2>&1";
        FILE* pipe = popen(full_cmd.c_str(), "r");
        if (!pipe) {
            if (out_status) *out_status = -127;
            return "";
        }
        std::string output;
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }
        int st = pclose(pipe);
        if (out_status) *out_status = WIFEXITED(st) ? WEXITSTATUS(st) : st;
        return output;
#endif
    }

    // Native C++ recursive-descent evaluator for safe Python math/stdlib expressions
    struct NativePyParser {
        const std::string& src;
        const std::unordered_map<std::string, Value>& vars;
        size_t pos = 0;

        NativePyParser(const std::string& s, const std::unordered_map<std::string, Value>& v)
            : src(s), vars(v) {}

        void skip_ws() {
            while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\r' || src[pos] == '\n')) {
                ++pos;
            }
        }

        bool match_str(const char* kw) {
            skip_ws();
            size_t len = std::strlen(kw);
            if (pos + len > src.size()) return false;
            if (src.compare(pos, len, kw) != 0) return false;
            if (std::isalpha(static_cast<unsigned char>(kw[len - 1])) || kw[len - 1] == '_') {
                if (pos + len < src.size() && (std::isalnum(static_cast<unsigned char>(src[pos + len])) || src[pos + len] == '_')) {
                    return false;
                }
            }
            pos += len;
            return true;
        }

        bool parse_full(Value& out) {
            if (!parse_or(out)) return false;
            skip_ws();
            return pos == src.size();
        }

        bool parse_or(Value& out) {
            if (!parse_and(out)) return false;
            while (match_str("or")) {
                Value rhs;
                if (!parse_and(rhs)) return false;
                out = Value(out.is_truthy() || rhs.is_truthy());
            }
            return true;
        }

        bool parse_and(Value& out) {
            if (!parse_not(out)) return false;
            while (match_str("and")) {
                Value rhs;
                if (!parse_not(rhs)) return false;
                out = Value(out.is_truthy() && rhs.is_truthy());
            }
            return true;
        }

        bool parse_not(Value& out) {
            if (match_str("not")) {
                Value sub;
                if (!parse_not(sub)) return false;
                out = Value(!sub.is_truthy());
                return true;
            }
            return parse_cmp(out);
        }

        bool parse_cmp(Value& out) {
            if (!parse_add(out)) return false;
            skip_ws();
            if (pos < src.size()) {
                std::string op;
                if (src.compare(pos, 2, "==") == 0 || src.compare(pos, 2, "!=") == 0 ||
                    src.compare(pos, 2, "<=") == 0 || src.compare(pos, 2, ">=") == 0) {
                    op = src.substr(pos, 2);
                    pos += 2;
                } else if (src[pos] == '<' || src[pos] == '>') {
                    op = std::string(1, src[pos]);
                    ++pos;
                }
                if (!op.empty()) {
                    Value rhs;
                    if (!parse_add(rhs)) return false;
                    if (op == "==") out = Value(out == rhs);
                    else if (op == "!=") out = Value(out != rhs);
                    else if (op == "<") out = Value(out < rhs);
                    else if (op == "<=") out = Value(out <= rhs);
                    else if (op == ">") out = Value(out > rhs);
                    else if (op == ">=") out = Value(out >= rhs);
                }
            }
            return true;
        }

        bool parse_add(Value& out) {
            if (!parse_mul(out)) return false;
            while (true) {
                skip_ws();
                if (pos >= src.size()) break;
                char c = src[pos];
                if (c != '+' && c != '-') break;
                ++pos;
                Value rhs;
                if (!parse_mul(rhs)) return false;
                out = (c == '+') ? (out + rhs) : (out - rhs);
            }
            return true;
        }

        bool parse_mul(Value& out) {
            if (!parse_unary(out)) return false;
            while (true) {
                skip_ws();
                if (pos >= src.size()) break;
                if (pos + 1 < src.size() && src[pos] == '*' && src[pos + 1] == '*') break;
                if (pos + 1 < src.size() && src[pos] == '/' && src[pos + 1] == '/') {
                    pos += 2;
                    Value rhs;
                    if (!parse_unary(rhs)) return false;
                    double b = rhs.as_number();
                    if (std::abs(b) < 1e-15) return false;
                    double q = std::floor(out.as_number() / b);
                    out = (out.type == ValueType::Int && rhs.type == ValueType::Int)
                              ? Value(static_cast<int64_t>(q))
                              : Value(q);
                    continue;
                }
                char c = src[pos];
                if (c != '*' && c != '/' && c != '%') break;
                ++pos;
                Value rhs;
                if (!parse_unary(rhs)) return false;
                if ((c == '/' || c == '%') && std::abs(rhs.as_number()) < 1e-15) return false;
                if (c == '*') out = out * rhs;
                else if (c == '/') {
                    double dv = out.as_number() / rhs.as_number();
                    out = Value(dv);
                } else {
                    out = out % rhs;
                }
            }
            return true;
        }

        bool parse_unary(Value& out) {
            skip_ws();
            if (pos < src.size() && (src[pos] == '+' || src[pos] == '-')) {
                char c = src[pos++];
                Value sub;
                if (!parse_unary(sub)) return false;
                out = (c == '-') ? (Value(0) - sub) : sub;
                return true;
            }
            return parse_pow(out);
        }

        bool parse_pow(Value& out) {
            if (!parse_primary(out)) return false;
            skip_ws();
            if (pos + 1 < src.size() && src[pos] == '*' && src[pos + 1] == '*') {
                pos += 2;
                Value exp;
                if (!parse_unary(exp)) return false;
                out = out.power(exp);
            }
            return true;
        }

        static Value make_num_val(double d, bool prefer_int = false) {
            if (prefer_int && std::isfinite(d) && std::abs(d - std::round(d)) < 1e-12 &&
                d >= -9e15 && d <= 9e15) {
                return Value(static_cast<int64_t>(std::llround(d)));
            }
            return Value(d);
        }

        bool eval_builtin_call(const std::string& fn, const std::vector<Value>& args, Value& out) {
            if (fn == "math.sqrt" && args.size() == 1) {
                double x = args[0].as_number();
                if (x < 0.0) return false;
                out = Value(std::sqrt(x));
                return true;
            }
            if (fn == "math.pow" && args.size() == 2) {
                out = Value(std::pow(args[0].as_number(), args[1].as_number()));
                return true;
            }
            if (fn == "math.sin" && args.size() == 1) { out = Value(std::sin(args[0].as_number())); return true; }
            if (fn == "math.cos" && args.size() == 1) { out = Value(std::cos(args[0].as_number())); return true; }
            if (fn == "math.tan" && args.size() == 1) { out = Value(std::tan(args[0].as_number())); return true; }
            if (fn == "math.asin" && args.size() == 1) { out = Value(std::asin(args[0].as_number())); return true; }
            if (fn == "math.acos" && args.size() == 1) { out = Value(std::acos(args[0].as_number())); return true; }
            if (fn == "math.atan" && args.size() == 1) { out = Value(std::atan(args[0].as_number())); return true; }
            if (fn == "math.atan2" && args.size() == 2) { out = Value(std::atan2(args[0].as_number(), args[1].as_number())); return true; }
            if (fn == "math.floor" && args.size() == 1) { out = make_num_val(std::floor(args[0].as_number()), true); return true; }
            if (fn == "math.ceil" && args.size() == 1) { out = make_num_val(std::ceil(args[0].as_number()), true); return true; }
            if (fn == "math.fabs" && args.size() == 1) { out = Value(std::fabs(args[0].as_number())); return true; }
            if (fn == "math.exp" && args.size() == 1) { out = Value(std::exp(args[0].as_number())); return true; }
            if (fn == "math.log" && args.size() == 1) {
                double x = args[0].as_number();
                if (x <= 0.0) return false;
                out = Value(std::log(x));
                return true;
            }
            if (fn == "math.log10" && args.size() == 1) {
                double x = args[0].as_number();
                if (x <= 0.0) return false;
                out = Value(std::log10(x));
                return true;
            }
            if (fn == "math.degrees" && args.size() == 1) { out = Value(args[0].as_number() * 180.0 / 3.14159265358979323846); return true; }
            if (fn == "math.radians" && args.size() == 1) { out = Value(args[0].as_number() * 3.14159265358979323846 / 180.0); return true; }
            if (fn == "math.hypot" && args.size() == 2) { out = Value(std::hypot(args[0].as_number(), args[1].as_number())); return true; }
            if (fn == "math.factorial" && args.size() == 1) {
                int64_t n = args[0].as_int();
                if (n < 0 || n > 20) return false;
                int64_t r = 1;
                for (int64_t k = 2; k <= n; ++k) r *= k;
                out = Value(r);
                return true;
            }
            if (fn == "round" && (args.size() == 1 || args.size() == 2)) {
                double x = args[0].as_number();
                if (args.size() == 1) {
                    out = make_num_val(std::round(x), true);
                    return true;
                }
                int digits = static_cast<int>(args[1].as_int());
                double factor = std::pow(10.0, digits);
                out = Value(std::round(x * factor) / factor);
                return true;
            }
            if (fn == "abs" && args.size() == 1) {
                if (args[0].type == ValueType::Int) out = Value(std::llabs(args[0].int_val));
                else out = Value(std::fabs(args[0].as_number()));
                return true;
            }
            if (fn == "min" && !args.empty()) {
                if (args.size() == 1 && args[0].type == ValueType::List && args[0].list_val && !args[0].list_val->empty()) {
                    Value best = (*args[0].list_val)[0];
                    for (const auto& el : *args[0].list_val) if (el < best) best = el;
                    out = best;
                    return true;
                }
                Value best = args[0];
                for (const auto& el : args) if (el < best) best = el;
                out = best;
                return true;
            }
            if (fn == "max" && !args.empty()) {
                if (args.size() == 1 && args[0].type == ValueType::List && args[0].list_val && !args[0].list_val->empty()) {
                    Value best = (*args[0].list_val)[0];
                    for (const auto& el : *args[0].list_val) if (best < el) best = el;
                    out = best;
                    return true;
                }
                Value best = args[0];
                for (const auto& el : args) if (best < el) best = el;
                out = best;
                return true;
            }
            if (fn == "int" && args.size() == 1) { out = Value(args[0].as_int()); return true; }
            if (fn == "float" && args.size() == 1) { out = Value(args[0].as_number()); return true; }
            if (fn == "bool" && args.size() == 1) { out = Value(args[0].is_truthy()); return true; }
            if (fn == "str" && args.size() == 1) { out = Value(args[0].as_string()); return true; }
            if (fn == "len" && args.size() == 1) { out = Value(static_cast<int64_t>(args[0].size())); return true; }
            if (fn == "sum" && args.size() == 1 && args[0].type == ValueType::List && args[0].list_val) {
                double s = 0.0;
                bool all_int = true;
                for (const auto& el : *args[0].list_val) {
                    if (el.type != ValueType::Int) all_int = false;
                    s += el.as_number();
                }
                out = make_num_val(s, all_int);
                return true;
            }
            return false;
        }

        bool parse_primary(Value& out) {
            skip_ws();
            if (pos >= src.size()) return false;

            if (src[pos] == '(') {
                ++pos;
                if (!parse_or(out)) return false;
                skip_ws();
                if (pos >= src.size() || src[pos] != ')') return false;
                ++pos;
                return true;
            }

            if (src[pos] == '[') {
                ++pos;
                std::vector<Value> elems;
                skip_ws();
                if (pos < src.size() && src[pos] == ']') {
                    ++pos;
                    out = Value(elems);
                    return true;
                }
                while (true) {
                    Value el;
                    if (!parse_or(el)) return false;
                    elems.push_back(el);
                    skip_ws();
                    if (pos < src.size() && src[pos] == ',') {
                        ++pos;
                        skip_ws();
                        if (pos < src.size() && src[pos] == ']') { ++pos; break; }
                    } else if (pos < src.size() && src[pos] == ']') {
                        ++pos;
                        break;
                    } else {
                        return false;
                    }
                }
                out = Value(elems);
                return true;
            }

            if (src[pos] == '"' || src[pos] == '\'') {
                char q = src[pos++];
                std::string s;
                while (pos < src.size() && src[pos] != q) {
                    if (src[pos] == '\\' && pos + 1 < src.size()) {
                        char esc = src[++pos];
                        if (esc == 'n') s += '\n';
                        else if (esc == 't') s += '\t';
                        else if (esc == 'r') s += '\r';
                        else s += esc;
                        ++pos;
                    } else {
                        s += src[pos++];
                    }
                }
                if (pos >= src.size() || src[pos] != q) return false;
                ++pos;
                out = Value(s);
                return true;
            }

            if (std::isdigit(static_cast<unsigned char>(src[pos])) ||
                (src[pos] == '.' && pos + 1 < src.size() && std::isdigit(static_cast<unsigned char>(src[pos + 1])))) {
                size_t start = pos;
                bool is_float = false;
                while (pos < src.size() && (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.')) {
                    if (src[pos] == '.') is_float = true;
                    ++pos;
                }
                if (pos < src.size() && (src[pos] == 'e' || src[pos] == 'E')) {
                    is_float = true;
                    ++pos;
                    if (pos < src.size() && (src[pos] == '+' || src[pos] == '-')) ++pos;
                    while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
                }
                std::string num_s = src.substr(start, pos - start);
                try {
                    if (is_float) out = Value(std::stod(num_s));
                    else out = Value(static_cast<int64_t>(std::stoll(num_s)));
                    return true;
                } catch (...) {
                    return false;
                }
            }

            if (std::isalpha(static_cast<unsigned char>(src[pos])) || src[pos] == '_') {
                size_t start = pos;
                while (pos < src.size() && (std::isalnum(static_cast<unsigned char>(src[pos])) || src[pos] == '_' || src[pos] == '.')) {
                    ++pos;
                }
                std::string ident = src.substr(start, pos - start);
                skip_ws();
                if (pos < src.size() && src[pos] == '(') {
                    ++pos;
                    std::vector<Value> args;
                    skip_ws();
                    if (pos < src.size() && src[pos] == ')') {
                        ++pos;
                    } else {
                        while (true) {
                            Value arg;
                            if (!parse_or(arg)) return false;
                            args.push_back(arg);
                            skip_ws();
                            if (pos < src.size() && src[pos] == ',') {
                                ++pos;
                            } else if (pos < src.size() && src[pos] == ')') {
                                ++pos;
                                break;
                            } else {
                                return false;
                            }
                        }
                    }
                    return eval_builtin_call(ident, args, out);
                }

                if (ident == "True") { out = Value(true); return true; }
                if (ident == "False") { out = Value(false); return true; }
                if (ident == "None") { out = Value(); return true; }
                if (ident == "math.pi") { out = Value(3.14159265358979323846); return true; }
                if (ident == "math.e") { out = Value(2.71828182845904523536); return true; }
                if (ident == "math.tau") { out = Value(6.28318530717958647692); return true; }
                if (ident == "math.inf") { out = Value(std::numeric_limits<double>::infinity()); return true; }

                auto vit = vars.find(ident);
                if (vit != vars.end()) {
                    out = vit->second;
                    return true;
                }
                return false;
            }

            return false;
        }
    };

    void parse_and_set_var(const std::string& json_str, std::unordered_map<std::string, Value>& variables) {
        // e.g. {"k": "target_xp", "v": 5400}
        auto k_pos = json_str.find("\"k\":");
        auto v_pos = json_str.find("\"v\":");
        if (k_pos == std::string::npos || v_pos == std::string::npos) return;

        // Extract key
        size_t k_start = json_str.find('"', k_pos + 4);
        if (k_start == std::string::npos) return;
        size_t k_end = json_str.find('"', k_start + 1);
        if (k_end == std::string::npos) return;
        std::string key = json_str.substr(k_start + 1, k_end - k_start - 1);

        // Extract value
        size_t v_start = v_pos + 4;
        while (v_start < json_str.size() && (json_str[v_start] == ' ' || json_str[v_start] == ':')) v_start++;
        size_t v_end = json_str.find_last_not_of(" }\r\n");
        if (v_end == std::string::npos || v_end < v_start) return;
        std::string raw_v = json_str.substr(v_start, v_end - v_start + 1);

        variables[key] = parse_json_value(raw_v);
    }

    Value parse_json_value(const std::string& s) {
        std::string trimmed = s;
        while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t')) trimmed.erase(0, 1);
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t' || trimmed.back() == '\n' || trimmed.back() == '\r')) trimmed.pop_back();

        if (trimmed == "true") return Value(true);
        if (trimmed == "false") return Value(false);
        if (trimmed == "null" || trimmed == "None") return Value();

        if (trimmed.size() >= 2 && trimmed.front() == '"' && trimmed.back() == '"') {
            return Value(trimmed.substr(1, trimmed.size() - 2));
        }

        // Try number
        try {
            if (trimmed.find('.') != std::string::npos) {
                return Value(std::stod(trimmed));
            } else {
                return Value(static_cast<int64_t>(std::stoll(trimmed)));
            }
        } catch (...) {}

        return Value(trimmed);
    }
};

} // namespace looping
