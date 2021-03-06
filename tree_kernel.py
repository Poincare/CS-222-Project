from sequitur_wrapper import Sequitur
from os import walk
import sys

class TreeKernel():
    def __init__(self, sequitur_tree_map):
        """Takes a Sequitur tree as returned by the Sequitur output parser."""
        self.sequitur_tree_map = sequitur_tree_map

    def get_svm_light(self, target):
        """Outputs sequitur tree map in a form that svm_light can consume."""
        output = "%s |BT| " % target

        tree_label = 'NODE'
        start_node = 0
        def tree_dfs(node, tree_repr):
            # dfs through the tree and turn it into the svm light form
            ret = " (%s" % tree_label
            children = tree_repr[node]
            for child in children:
                try:
                    assert (child in tree_repr)
                    child_ret = tree_dfs(child, tree_repr)
                    ret += child_ret
                except AssertionError:
                    leaf_ret = " (%s %s)" % (tree_label, child)
                    ret += leaf_ret

            ret += ")"
            return ret

        output += tree_dfs('0', self.sequitur_tree_map)
        output += " |ET|"
        return output

# Potential questions we should address in the paper:
# Should we do the breakup by sentence? What about by the number of characters?
# Is there a better feature than just the number of ham sentences in a particular email
# that we can derive from the tree-kernel classifier that would be more helpful?
def output_trees(directory, target):
    limit = 1000
    i = 0
    for (dirpath, dirnames, filenames) in walk(directory):
        for filename in filenames:
            if i > limit:
                break

            # sys.stderr.write("File #%d: %s : %d\n" % (i, filename, target))
            
            # break up the file into sentences and construct trees for each sentence
            sentences = []
            for line in open(directory + filename, 'r'):
                sentences += line.split('.')

            for sentence in sentences:
                # have to write the buffer to a file in order for Sequitur Simple to
                # be able to use it.
                write_buffer_name = 'sentence_buffer'
                write_buffer = open(write_buffer_name, 'w')
                write_buffer.write(sentence + "\n")
                
                seq = Sequitur(write_buffer_name)
                tk = TreeKernel(seq.compute_grammar())
                print tk.get_svm_light(target)
                i += 1
        break

ham_directory = "/Users/dhaivat/dev/harvard/cs222/CS-222-Project/enron1/ham/"
spam_directory = "/Users/dhaivat/dev/harvard/cs222/CS-222-Project/enron1/spam/"
output_trees(ham_directory, 1)
output_trees(spam_directory, -1)
#number of files per classification (i.e. per spam or ham)
