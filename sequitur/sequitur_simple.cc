#include <iostream>
#include <bits/stdc++.h>
#include <vector>
#include <math.h>

using namespace std;

typedef unsigned long ulong;

// This code was written for didactic purposes;
// it is feature-poor in order to be as simple as possible.
// For a more realistic implementation of Sequitur, please use
// http://sequitur.info/latest/sequitur.tgz

class symbols;
class rules;
class features;

// This finds a matching digram in the grammar, and returns a pointer
// to its entry in the hash table so it can be changed or deleted as necessary

extern symbols **find_digram(symbols *s);

///////////////////////////////////////////////////////////////////////////

class rules {
  // the guard node in the linked list of symbols that make up the rule
  // It points forward to the first symbol in the rule, and backwards
  // to the last symbol in the rule. Its own value points to the rule data
  // structure, so that symbols can find out which rule they're in

  symbols *guard;

  // this is just for numbering the rules nicely for printing; it's
  // not essential for the algorithm
  int number;

public:
  rules();
  ~rules();

  //  count keeps track of the number of times the rule is used in the grammar
  int count;
    
  void reuse() { count ++; }
  void deuse() { count --; };

  symbols *first();
  symbols *last();

  int freq() { return count; };
  int index() { return number; };
  void index(int i) { number = i; };
};

// Stores features computed on output Sequitur grammars for ML analysis

class features {
public:
    features();
    ~features();

    //the number of rules in the grammar
    int num_rules;
    //average symbols per rule
    double average_rule_length;
    //average number of times a rule is used
    double average_rule_usage;
    
    double stddev_rule_length;
    double stddev_rule_usage;
    
    //proportion of terminal characters in grammar
    //this feature and the average_rule_usage feature may be measuring very
    //similar things, so I think only one of these is necessary (Matthew)
    double proportion_terminal_characters;
    
    //rules that are of the form R -> x y, where x,y are any symbols
    double proportion_bigrams;
    
    //rules that are of the form R -> R' R', where R' is another form
    double proportion_repeat_rule_bigrams;
    //rules that are of the form R -> X X, where X is another symbol
    double proportion_repeat_bigrams;
    
    int* ptr_num_rules() { return &num_rules; };
    double* ptr_average_rule_length() { return &average_rule_length; };
    double* ptr_average_rule_usage() { return &average_rule_usage; };
    double* ptr_stddev_rule_usage()
      {return &stddev_rule_usage; };
    double* ptr_stddev_rule_length()
      {return &stddev_rule_length; };
    double* ptr_proportion_terminal_characters()
      { return &proportion_terminal_characters; };
    double* ptr_proportion_bigrams()
      { return &proportion_bigrams; };
    double* ptr_proportion_repeat_rule_bigrams()
      { return &proportion_repeat_rule_bigrams; };
    double* ptr_proportion_repeat_bigrams()
      { return &proportion_repeat_bigrams; };

    void print() {
        /**cout << "num_rules: " << num_rules << endl;
        cout << "average_rule_length: " << average_rule_length << endl;
        cout << "average_rule_usage: " << average_rule_usage << endl;
        cout << "proportion_terminal_characters: " <<
          proportion_terminal_characters << endl;**/
        ofstream mydata;
        mydata.open("seq_features_all.csv", ios_base::app);
        // cout << "num_rules,average_rule_length," <<
        // "average_rule_usage,proportional_terminal_characters" << endl;
        mydata << num_rules << "," << average_rule_length << "," <<
          average_rule_usage << "," << stddev_rule_length << "," <<
          stddev_rule_usage << "," << proportion_terminal_characters << "," <<
          proportion_bigrams << "," << proportion_repeat_rule_bigrams << "," <<
          proportion_repeat_bigrams << 1 << endl;
    }
};

features::features() {
    num_rules = 0;
    average_rule_length = 0;
    average_rule_usage = 0;
    stddev_rule_length = 0;
    stddev_rule_usage = 0;
    proportion_terminal_characters = 0;
    proportion_bigrams = 0;
    proportion_repeat_rule_bigrams = 0;
    proportion_repeat_bigrams = 0;
}

class symbols {
  symbols *n, *p;
  ulong s;

 public:

  // initializes a new symbol. If it is non-terminal, increments the reference
  // count of the corresponding rule

  symbols(ulong sym) {
    s = sym * 2 + 1;
    // an odd number, so that they're distinct from the rule pointers, which
    // we assume are 4-byte aligned
    p = n = 0;
  }

  symbols(rules *r) {
    s = (ulong) r;
    p = n = 0;
    rule()->reuse();
  }

  // links two symbols together, removing any old digram from the hash table

  static void join(symbols *left, symbols *right) {
    if (left->n) {
      left->delete_digram();

      // This is to deal with triples, where we only record the second
      // pair of the overlapping digrams. When we delete the second pair,
      // we insert the first pair into the hash table so that we don't
      // forget about it.  e.g. abbbabcbb

      if (right->p && right->n &&
          right->value() == right->p->value() &&
          right->value() == right->n->value()) {
        *find_digram(right) = right;
      }

      if (left->p && left->n &&
          left->value() == left->n->value() &&
          left->value() == left->p->value()) {
        *find_digram(left->p) = left->p;
      }
    }
    left->n = right; right->p = left;
  }

  // cleans up for symbol deletion: removes hash table entry and decrements
  // rule reference count

  ~symbols() {
    join(p, n);
    if (!is_guard()) {
      delete_digram();
      if (non_terminal()) rule()->deuse();
    }
  }

  // inserts a symbol after this one.

  void insert_after(symbols *y) {
    join(y, n);
    join(this, y);
  };

  // removes the digram from the hash table

  void delete_digram() {
    if (is_guard() || n->is_guard()) return;
    symbols **m = find_digram(this);
    if (*m == this) *m = (symbols *) 1;
  }

  // returns true if this is the guard node marking the beginning/end of a rule

  int is_guard() { return non_terminal() && rule()->first()->prev() == this; };

  // non_terminal() returns true if a symbol is non-terminal.
  // If s is odd, the symbol is a terminal
  // If s is even, the symbol is  non-terminal: a pointer to the referenced rule

  int non_terminal() { return ((s % 2) == 0) && (s != 0);};

  symbols *next() { return n;};
  symbols *prev() { return p;};
  inline ulong raw_value() {  return s; };
  inline ulong value() { return s / 2;};

  // assuming this is a non-terminal, rule() returns the corresponding rule

  rules *rule() { return (rules *) s;};

  void substitute(rules *r);
  static void match(symbols *s, symbols *m);

  // checks a new digram. If it appears elsewhere, deals with it by calling
  // match(), otherwise inserts it into the hash table

  int check() {
    if (is_guard() || n->is_guard()) return 0;
    symbols **x = find_digram(this);
    if (ulong(*x) <= 1) {
      *x = this;
      return 0;
    }

    if (ulong(*x) > 1 && (*x)->next() != this) match(this, *x);
    return 1;
  }

  void expand();

  void point_to_self() { join(this, this); };
};

rules S;
//num characters in file
int total_num_characters;

int main()
{
  S.last()->insert_after(new symbols(cin.get()));
  int x = 0;
  total_num_characters = 0;
  while (1) {
    total_num_characters++;
    int i = cin.get(); if (cin.eof()) break;
    S.last()->insert_after(new symbols(i));
    S.last()->prev()->check();
  }

  void print();
  print();
}

int num_rules = 0;

rules::rules()
{
  num_rules ++;
  guard = new symbols(this);
  guard->point_to_self();
  count = number = 0;
}

rules::~rules() {
  num_rules --;
  delete guard;
}

symbols *rules::first() { return guard->next(); }
symbols *rules::last() { return guard->prev(); }

// This symbol is the last reference to its rule. It is deleted, and the
// contents of the rule substituted in its place.

void symbols::expand() {
  symbols *left = prev();
  symbols *right = next();
  symbols *f = rule()->first();
  symbols *l = rule()->last();

  delete rule();
  symbols **m = find_digram(this);
  if (*m == this) *m = (symbols *) 1;
  s = 0; // if we don't do this, deleting the symbol tries to deuse the rule!
  delete this;

  join(left, f);
  join(l, right);

  *find_digram(l) = l;
}

// Replace a digram with a non-terminal

void symbols::substitute(rules *r)
{
  symbols *q = p;

  delete q->next();
  delete q->next();

  q->insert_after(new symbols(r));

  if (!q->check()) q->n->check();
}

// Deal with a matching digram

void symbols::match(symbols *ss, symbols *m)
{
  rules *r;

  // reuse an existing rule
  if (m->prev()->is_guard() && m->next()->next()->is_guard()) {
    r = m->prev()->rule();
    ss->substitute(r);
  }
  else {
    // create a new rule
    r = new rules;

    if (ss->non_terminal())
      r->last()->insert_after(new symbols(ss->rule()));
    else
      r->last()->insert_after(new symbols(ss->value()));

    if (ss->next()->non_terminal())
      r->last()->insert_after(new symbols(ss->next()->rule()));
    else
      r->last()->insert_after(new symbols(ss->next()->value()));

    m->substitute(r);
    ss->substitute(r);

    *find_digram(r->first()) = r->first();
  }

  // check for an underused rule

  if (r->first()->non_terminal() && r->first()->rule()->freq() == 1) r->first()->expand();
}

// pick a prime! (large enough to hold every digram in the grammar, with room
// to spare

#define PRIME 2265539

// Standard open addressing or double hashing. See Knuth.

#define HASH(one, two) (((one) << 16 | (two)) % PRIME)
#define HASH2(one) (17 - ((one) % 17))

symbols *table[PRIME];

symbols **find_digram(symbols *s)
{
  ulong one = s->raw_value();
  ulong two = s->next()->raw_value();

  int jump = HASH2(one);
  int insert = -1;
  int i = HASH(one, two);

  while (1) {
    symbols *m = table[i];
    if (!m) {
      if (insert == -1) insert = i;
      return &table[insert];
    }
    else if (ulong(m) == 1) insert = i;
    else if (m->raw_value() == one && m->next()->raw_value() == two)
      return &table[i];
    i = (i + jump) % PRIME;
  }
}

// print the rules out

rules **R;
int Ri;
vector<int> rule_lengths;
vector<int> rule_usages;

void p(rules *r, features *f) {
    int curr_rule_length = 0;
    //2 symbols in rule
    if(r->last() == r->first()->next()) {
        (*f->ptr_proportion_bigrams())++;
        symbols *first = r->first();
        symbols *last = r->last();
        if(first->non_terminal() && last->non_terminal()) {
            if(first->rule()->index() == last->rule()->index()) {
                (*f->ptr_proportion_repeat_bigrams())++;
                (*f->ptr_proportion_repeat_rule_bigrams())++;
            }
        }
        else if (!first->non_terminal() && !last->non_terminal()) {
            if(first->value() == last->value()) {
                (*f->ptr_proportion_repeat_bigrams())++;
            }
        }
    }
    for (symbols *p = r->first(); !p->is_guard(); p = p->next()) {
        if (p->non_terminal()) {
            int i;

            if (R[p->rule()->index()] == p->rule())
                i = p->rule()->index();
            else {
                i = Ri;
                p->rule()->index(Ri);
                R[Ri ++] = p->rule();
            }

            cout << i << ' ';
        }
        else {
            if (p->value() == ' ') cout << '_';
            else if (p->value() == '\n') cout << "\\n";
            else if (p->value() == '\t') cout << "\\t";
            else if (p->value() == '\\' ||
                     p->value() == '(' ||
                     p->value() == ')' ||
                     p->value() == '_' ||
                     isdigit(p->value()))
                cout << '\\' << char(p->value());
            else cout << char(p->value());
            cout << ' ';
            (*f->ptr_proportion_terminal_characters()) ++;
        }
        curr_rule_length++;
    }
    rule_lengths.push_back(curr_rule_length);
    rule_usages.push_back(r->count);
    cout << endl;
}

void print()
{
    R = (rules **) malloc(sizeof(rules *) * num_rules);
    memset(R, 0, sizeof(rules *) * num_rules);
    R[0] = &S;
    Ri = 1;

    features *f = new features;

    for (int i = 0; i < Ri; i ++) {
      cout << i << " -> ";
      p(R[i], f);
    }
    
    double avg_rule_length = 0;
    double avg_rule_usage = 0;

    for(int i = 0; i < rule_lengths.size(); ++i) {
        avg_rule_length += rule_lengths[i];
    }
    for(int i = 0; i < rule_usages.size(); ++i) {
        avg_rule_usage += rule_usages[i];
    }
    avg_rule_length /= (double) num_rules;
    //rules are used in their own definition
    avg_rule_usage += num_rules;
    avg_rule_usage /= (double) num_rules;
    
    double stddev_length = 0;
    double stddev_usage = 0;
    for(int i = 0; i < rule_lengths.size(); ++i) {
        stddev_length += pow(avg_rule_length - rule_lengths[i], 2.0);
    }
    
    for(int i = 0; i < rule_usages.size(); ++i) {
        stddev_usage += pow(avg_rule_usage - rule_usages[i], 2.0);
    }
    
    stddev_length = sqrt(stddev_length / (double) num_rules);
    stddev_usage = sqrt(stddev_usage / (double) num_rules);

    *(f->ptr_num_rules()) = num_rules;

    *(f->ptr_proportion_terminal_characters()) /= total_num_characters;
    
    *(f->ptr_average_rule_length()) = avg_rule_length;
    *(f->ptr_average_rule_usage()) = avg_rule_usage;
    
    *(f->ptr_stddev_rule_length()) = stddev_length;
    *(f->ptr_stddev_rule_usage()) = stddev_usage;

    *(f->ptr_proportion_bigrams()) /= (double) num_rules;
    *(f->ptr_proportion_repeat_bigrams()) /= (double) num_rules;
    *(f->ptr_proportion_repeat_rule_bigrams()) /= (double) num_rules;
    
    f->print();
    free(R);
}
