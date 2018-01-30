#include <cstring>

#if defined(_WIN32)
#include <Windows.h>

template <typename T>
void zm(T& data)
{
    ZeroMemory(&data, sizeof(T));
}

int exec(const char* cmd)
{
    const short lim = _MAX_PATH + 10;
    char cmd_line[lim];
    strcpy(cmd_line, "cmd /c ");
    strncat(cmd_line, cmd, _MAX_PATH);

    STARTUPINFO s_info;
    zm(s_info);
    s_info.cb = sizeof(STARTUPINFO);
    s_info.dwFlags = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION proc_info;
    zm(proc_info);

    auto success = CreateProcessA(
        nullptr,
        &cmd_line[0],
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &s_info,
        &proc_info);

    if (!success) return -1;

    // spin lock? doesn't seem to do anything different
    // while (WAIT_TIMEOUT == WaitForSingleObject(proc_info.hProcess, 0));
    WaitForSingleObject(proc_info.hProcess, INFINITE);

    DWORD exit_code;
    success = GetExitCodeProcess(proc_info.hProcess, &exit_code);

    if (!success) return -1;

    CloseHandle(proc_info.hProcess);
    CloseHandle(proc_info.hThread);

    return int(exit_code);
}

#else
#endif

#define PICOBENCH_DEBUG
#define PICOBENCH_IMPLEMENT
#include "picobench/picobench.hpp"

using namespace picobench;
using namespace std;

// calculate nanoseconds to spawn an empty process
// by running some empty commands and taking the minimum
int64_t calc_spawn_time()
{
    const int lim = 50;
    int64_t min_sample = LLONG_MAX;
    for (int i = 0; i < lim; ++i)
    {
        auto start = high_res_clock::now();
        exec("");
        auto duration = high_res_clock::now() - start;
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        if (ns < min_sample) min_sample = ns;
    }
    return min_sample;
}

struct bench
{
    string name;
    string cmd;
};

vector<bench> benchmarks;
int64_t spawn_time;

void bench_proc(state& s)
{
    const char* cmd = benchmarks[s.user_data()].cmd.c_str();
    for (auto _ : s)
    {
        exec(cmd);
    }

    s.add_custom_duration(-spawn_time);
};

int main(int argc, char* argv[])
{    
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
        {
            benchmarks.push_back({ argv[i], argv[i] });
        }
    }

    for (size_t i = 0; i < benchmarks.size(); ++i)
    {
        auto& b = benchmarks[i];
        registry::new_benchmark(b.name.c_str(), bench_proc).user_data(i);
    }

    runner r;
    r.set_default_state_iterations({ 1 });
    r.parse_cmd_line(argc, argv);
    if (r.should_run())
    {
        spawn_time = calc_spawn_time();

        auto report = r.run_benchmarks();
        std::ostream* out = &std::cout;
        std::ofstream fout;
        if (r.preferred_output_filename())
        {
            fout.open(r.preferred_output_filename());
            if (!fout.is_open())
            {
                std::cerr << "Error: Could not open output file `" << r.preferred_output_filename() << "`\n";
                return 1;
            }
            out = &fout;
        }

        switch (r.preferred_output_format())
        {
        case picobench::report_output_format::text:
            report.to_text(*out);
            break;
        case picobench::report_output_format::concise_text:
            report.to_text_concise(*out);
            break;
        case picobench::report_output_format::csv:
            report.to_csv(*out);
            break;
        }
    }
    return r.error();
    
    return 0;
}
