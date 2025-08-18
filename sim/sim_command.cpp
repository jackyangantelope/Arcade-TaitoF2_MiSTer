#include "sim_command.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <getopt.h>
#include <cstring>

void CommandQueue::add(const Command& cmd)
{
    commands.push(cmd);
    if (cmd.type != CommandType::EXIT)
        batch_mode = true;
}

bool CommandQueue::parse_arguments(int argc, char** argv, std::string& game_name)
{
    static struct option long_options[] = {
        {"load-state", required_argument, 0, 'l'},
        {"save-state", required_argument, 0, 's'},
        {"run-cycles", required_argument, 0, 'c'},
        {"run-frames", required_argument, 0, 'f'},
        {"screenshot", required_argument, 0, 'p'},
        {"trace-start", required_argument, 0, 't'},
        {"trace-stop", no_argument, 0, 'T'},
        {"script", required_argument, 0, 'x'},
        {"headless", no_argument, 0, 'h'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    // Reset getopt
    optind = 1;
    
    while ((c = getopt_long(argc, argv, "l:s:c:f:p:t:Tx:hv?", long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'l':
            add(Command(CommandType::LOAD_STATE, optarg));
            if (verbose) std::cout << "Command: Load state from " << optarg << std::endl;
            break;
            
        case 's':
            add(Command(CommandType::SAVE_STATE, optarg));
            if (verbose) std::cout << "Command: Save state to " << optarg << std::endl;
            break;
            
        case 'c':
            {
                uint64_t cycles = std::stoull(optarg);
                add(Command(CommandType::RUN_CYCLES, cycles));
                if (verbose) std::cout << "Command: Run for " << cycles << " cycles" << std::endl;
            }
            break;
            
        case 'f':
            {
                uint64_t frames = std::stoull(optarg);
                add(Command(CommandType::RUN_FRAMES, frames));
                if (verbose) std::cout << "Command: Run for " << frames << " frames" << std::endl;
            }
            break;
            
        case 'p':
            add(Command(CommandType::SCREENSHOT, optarg));
            if (verbose) std::cout << "Command: Save screenshot to " << optarg << std::endl;
            break;
            
        case 't':
            add(Command(CommandType::TRACE_START, optarg));
            if (verbose) std::cout << "Command: Start trace to " << optarg << std::endl;
            break;
            
        case 'T':
            add(Command(CommandType::TRACE_STOP));
            if (verbose) std::cout << "Command: Stop trace" << std::endl;
            break;
            
        case 'x':
            if (!parse_script_file(optarg))
            {
                std::cerr << "Error: Failed to parse script file: " << optarg << std::endl;
                return false;
            }
            break;
            
        case 'h':
            headless = true;
            if (verbose) std::cout << "Running in headless mode" << std::endl;
            break;
            
        case 'v':
            verbose = true;
            std::cout << "Verbose mode enabled" << std::endl;
            break;
            
        case '?':
            print_usage(argv[0]);
            exit(0);
            break;
            
        default:
            std::cerr << "Unknown option: " << c << std::endl;
            print_usage(argv[0]);
            return false;
        }
    }
    
    // Get game name (positional argument)
    if (optind < argc)
    {
        game_name = argv[optind];
    }
    else if (!batch_mode)
    {
        game_name = "finalb";  // Default game
    }
    
    // Add implicit exit only for headless mode
    if (headless)
    {
        add(Command(CommandType::EXIT));
    }
    
    return true;
}

bool CommandQueue::parse_script_file(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open script file: " << filename << std::endl;
        return false;
    }
    
    if (verbose) std::cout << "Parsing script file: " << filename << std::endl;
    
    std::string line;
    int line_num = 0;
    
    while (std::getline(file, line))
    {
        line_num++;
        
        // Skip empty lines and comments
        size_t first_non_space = line.find_first_not_of(" \t\r\n");
        if (first_non_space == std::string::npos || line[first_non_space] == '#')
            continue;
            
        if (!parse_script_line(line))
        {
            std::cerr << "Error in script file " << filename << " at line " << line_num << ": " << line << std::endl;
            return false;
        }
    }
    
    return true;
}

bool CommandQueue::parse_script_line(const std::string& line)
{
    std::istringstream iss(line);
    std::string command;
    iss >> command;
    
    if (command == "load-state" || command == "load_state")
    {
        std::string filename;
        iss >> filename;
        if (filename.empty()) return false;
        add(Command(CommandType::LOAD_STATE, filename));
        if (verbose) std::cout << "Script: Load state from " << filename << std::endl;
    }
    else if (command == "save-state" || command == "save_state")
    {
        std::string filename;
        iss >> filename;
        if (filename.empty()) return false;
        add(Command(CommandType::SAVE_STATE, filename));
        if (verbose) std::cout << "Script: Save state to " << filename << std::endl;
    }
    else if (command == "run-cycles" || command == "run_cycles")
    {
        uint64_t cycles;
        iss >> cycles;
        if (iss.fail()) return false;
        add(Command(CommandType::RUN_CYCLES, cycles));
        if (verbose) std::cout << "Script: Run for " << cycles << " cycles" << std::endl;
    }
    else if (command == "run-frames" || command == "run_frames")
    {
        uint64_t frames;
        iss >> frames;
        if (iss.fail()) return false;
        add(Command(CommandType::RUN_FRAMES, frames));
        if (verbose) std::cout << "Script: Run for " << frames << " frames" << std::endl;
    }
    else if (command == "screenshot")
    {
        std::string filename;
        iss >> filename;
        if (filename.empty()) return false;
        add(Command(CommandType::SCREENSHOT, filename));
        if (verbose) std::cout << "Script: Save screenshot to " << filename << std::endl;
    }
    else if (command == "trace-start" || command == "trace_start")
    {
        std::string filename;
        iss >> filename;
        if (filename.empty()) return false;
        add(Command(CommandType::TRACE_START, filename));
        if (verbose) std::cout << "Script: Start trace to " << filename << std::endl;
    }
    else if (command == "trace-stop" || command == "trace_stop")
    {
        add(Command(CommandType::TRACE_STOP));
        if (verbose) std::cout << "Script: Stop trace" << std::endl;
    }
    else if (command == "wait" || command == "delay")
    {
        uint64_t ms;
        iss >> ms;
        if (iss.fail()) return false;
        // Convert milliseconds to cycles (assuming 12MHz)
        uint64_t cycles = ms * 12000;
        add(Command(CommandType::RUN_CYCLES, cycles));
        if (verbose) std::cout << "Script: Wait for " << ms << " ms (" << cycles << " cycles)" << std::endl;
    }
    else
    {
        std::cerr << "Unknown command: " << command << std::endl;
        return false;
    }
    
    return true;
}

void CommandQueue::print_usage(const char* program_name)
{
    std::cout << "Usage: " << program_name << " [options] [game_name]\n"
              << "\nOptions:\n"
              << "  --load-state <file>    Load savestate from file\n"
              << "  --save-state <file>    Save current state to file\n"
              << "  --run-cycles <N>       Run simulation for N cycles\n"
              << "  --run-frames <N>       Run simulation for N frames\n"
              << "  --screenshot <file>    Save screenshot to file\n"
              << "  --trace-start <file>   Start FST trace to file\n"
              << "  --trace-stop           Stop FST trace\n"
              << "  --script <file>        Execute commands from script file\n"
              << "  --headless             Run without GUI (batch mode only)\n"
              << "  --verbose              Print command execution details\n"
              << "  --help                 Show this help message\n"
              << "\nScript file format:\n"
              << "  # Comments start with #\n"
              << "  load-state checkpoint.state\n"
              << "  run-frames 100\n"
              << "  trace-start debug.fst\n"
              << "  run-frames 50\n"
              << "  trace-stop\n"
              << "  screenshot test.png\n"
              << "  save-state final.state\n"
              << "\nExample:\n"
              << "  " << program_name << " finalb --load-state test.state --run-frames 60 --screenshot out.png\n"
              << "  " << program_name << " driftout --script test_sequence.txt\n";
}