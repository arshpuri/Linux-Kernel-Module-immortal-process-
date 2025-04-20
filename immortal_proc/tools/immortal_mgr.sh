#!/bin/bash

function usage {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -l, --list        List protected processes"
    echo "  -a, --add PID     Protect process with PID"
    echo "  -r, --remove PID  Remove protection from process with PID"
    echo "  -s, --status      Show module status and statistics"
    echo "  -h, --help        Show this help"
    exit 1
}

function check_module {
    if ! lsmod | grep -q immortal_proc; then
        echo "Error: Immortal Process module is not loaded."
        echo "Load it with: sudo modprobe immortal_proc"
        exit 1
    fi
    
    if [ ! -e /proc/immortal_proc ]; then
        echo "Error: /proc/immortal_proc interface not found."
        exit 1
    fi
}

function list_procs {
    check_module
    cat /proc/immortal_proc
}

function add_proc {
    check_module
    if [ -z "$1" ]; then
        echo "Error: PID required"
        usage
    fi
    
    if ! ps -p "$1" > /dev/null; then
        echo "Error: Process $1 does not exist"
        exit 1
    fi
    
    echo "+$1" > /proc/immortal_proc
    echo "Process $1 is now protected"
}

function remove_proc {
    check_module
    if [ -z "$1" ]; then
        echo "Error: PID required"
        usage
    fi
    
    echo "-$1" > /proc/immortal_proc
    echo "Protection removed from process $1"
}

# Process command line arguments
case "$1" in
    -l|--list)
        list_procs
        ;;
    -a|--add)
        add_proc "$2"
        ;;
    -r|--remove)
        remove_proc "$2"
        ;;
    -s|--status)
        list_procs
        ;;
    -h|--help|*)
        usage
        ;;
esac
