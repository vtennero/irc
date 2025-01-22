import os
import sys
from basic_tests import run_basic_tests
from message_tests import run_message_tests
from stress_tests import run_stress_tests

def main():
    test_suites = [
        ("Basic Connection Tests", run_basic_tests),
        ("Message and Channel Tests", run_message_tests),
        ("Stress and Edge Case Tests", run_stress_tests)
    ]

    print("IRC Server Test Suite")
    print("====================")
    print("\nThis test suite will test various aspects of the IRC server.")
    print("The server should be running on localhost:6667 with password 'password123'")

    for i, (name, test_func) in enumerate(test_suites, 1):
        print(f"\n{i}. {name}")
        input("Press Enter to start this test suite (Ctrl+C to quit)...")
        try:
            test_func()
        except KeyboardInterrupt:
            print("\nTest suite interrupted by user")
            sys.exit(0)
        except Exception as e:
            print(f"Error running test suite: {e}")

        print(f"\n{name} completed.")
        if i < len(test_suites):
            input("Press Enter to continue to next test suite...")

    print("\nAll test suites completed!")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nTest runner interrupted by user")
        sys.exit(0)
