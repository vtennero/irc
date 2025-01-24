#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Array of test files
tests=(
    "basic_tests.py"
    "boundary_test.py"
    "concurrent_test.py"
    "long_line_test.py"
    "message_tests.py"
    "stress_tests.py"
)

# Function to run each test
run_test() {
    echo -e "\n${BLUE}Running test: ${GREEN}$1${NC}"
    python3 "tests/$1"

    # Check if the test passed
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Test passed: $1${NC}"
    else
        echo -e "\033[0;31mTest failed: $1${NC}"
    fi

    # Prompt for continuation
    echo -e "\nPress Enter to continue with the next test (or Ctrl+C to exit)..."
    read
}

# Print header
echo -e "${BLUE}Starting test suite execution...${NC}"
echo -e "${BLUE}Found ${#tests[@]} tests to run${NC}"

# Run each test
for test in "${tests[@]}"; do
    run_test "$test"
done

echo -e "\n${GREEN}All tests have been executed!${NC}"
