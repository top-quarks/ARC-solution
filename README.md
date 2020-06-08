Hello!

Below you can find a outline of how to run my solution to Kaggle's Abstraction and Reasoning Challenge.
If you run into any trouble with the setup/code or have any questions you can contact me at top-quarks@protonmail.com

## Tested computer setup
 - Ubuntu 18.04.4 LTS
 - Intel® Core™ i7-7700HQ CPU @ 2.80GHz × 8
 - 16GB RAM
 - Python 3.6.9
 - g++ 7.5.0

Any Python 3 and g++ supporting c++17 should work.

## Running on public data
The comptition data is already in the "dataset" folder for conveniece.

You can run the model on the evaluation dataset using depth 2 with (takes 70 seconds on my computer):

```python3 run.py```

You can view a summary of the results with:

```python3 summary.py```

It should give 129 / 419 correct predictions.

To run using depth 3, change "run_depth" to 3 in run.py<br/>
To run on the training dataset, change "sample_dir" to "training" on line 79 in src/runner.cpp, and set "inds = range(0,416)" in summary.py


## Running on test data
To run the full model and produce precictions on the test set (takes 9 hours), change "eval" to 1 on line 75 on src/runner.cpp and run

```python3 safe_run.py```

This produces the output file named "submission_part.csv", which can be renamed to "submission.csv" to submit to the competition.