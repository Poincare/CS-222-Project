# CS-222-Project

In order to run the `svm_light` tree kernel stuff, you should first change `tree_kernel.py` so that it reflects where the spam and ham directories for enron1 are located on your drive (see last three lines of the file).

Then, run the following:

```
python tree_kernel.py > ham_tree.train
sh svm_light_testing.sh
```
