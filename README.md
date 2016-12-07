# CS-222-Project
Feature extraction usage is below. *NOTE:* Before compiling sequitur_simple.cc, adjust the output to print 0 or 1 in the last column depending on if you're iterating through the ham or spam directory in import_features.sh. 

```bash
cd sequitur_simple
g++ -std=c++11 sequitur_simple.cc -o sequitur_simple
./import_features.sh
```


In order to run the `svm_light` tree kernel stuff, you should first change `tree_kernel.py` so that it reflects where the spam and ham directories for enron1 are located on your drive (see last three lines of the file).

Then, run the following:

```
python tree_kernel.py > ham_tree.train
sh svm_light_testing.sh
```
