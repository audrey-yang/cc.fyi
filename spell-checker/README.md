# Spell Checker Using a Bloom Filter

Following the [Build Your Own Spell Checker Using A Bloom Filter](https://codingchallenges.fyi/challenges/challenge-bloom) challenge.

## How to run

1. Prepare a dictionary. On Mac, you can run `cat /usr/share/dict/words >> dict.txt`.
1. Compile the project with `bazel build ...`.
1. First, build the archive file with `bazel run //src/main:main -- -build /path/to/dict.txt`
1. Then, run the spell checker with `bazel run //src/main:main -- list of words`.

Note: the archive is saved as `words.bf`. The path is currently hard-coded, as I have yet to figure out the pathing that Bazel uses. You will need to change the path in order to successfully run this program.

## Running tests

Run the tests with `bazel test --test_output=all //tests:test`.
