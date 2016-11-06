import subprocess

class Sequitur(object):
  def __init__(self, input_filename):
    self.input_filename = input_filename

  def parse_sequitur_output(self, output):
    grammar = {}
    for line in output:
      line = line.rstrip()
      rule_name, values = line.split('->')
      rule_name = rule_name.strip()
      grammar[rule_name] = values.lstrip().split(' ')
    return grammar

  def compute_grammar(self):
    process = subprocess.Popen(['./sequitur/sequitur_simple'],
      stdin=open(self.input_filename),
      stdout=open('buffer', 'w'))
    process.communicate()
    buffer = open('buffer', 'r')
    return self.parse_sequitur_output(buffer.readlines())


s = Sequitur('/Users/dhaivat/nltk_data/corpora/gutenberg/austen-emma.txt')
#s = Sequitur('test.txt')
print s.compute_grammar()
