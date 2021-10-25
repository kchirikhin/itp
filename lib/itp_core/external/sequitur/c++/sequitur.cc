/****************************************************************************

 sequitur.cc - Module containing the main() function, and functions for
               printing out the grammar.

    main() function contains: command line options parsing,
    program initialization, reading the input...

 Compilation notes:
    Define PLATFORM_MSWIN macro if compiling this program under Windows,
    or PLATFORM_UNIX if compiling it under Unix/Linux.

 Program usage (syntax):
    See "help" string below (or run "sequitur -h" after compiling the program).

 Additional notes:
    Progress indicator under MS Windows is yet to be done.

 ***************************************************************************/



#include <unistd.h>
#include <sys/timeb.h>

#ifdef PLATFORM_UNIX
#include <sys/times.h>
#endif

#include "classes.h"


class Sequitur
{
public:
	size_t compress(const unsigned char *src, size_t size)
	{
		if (!size)
		{
			return 0;
		}

		// number of input characters read so far (used only for progress indicator)
		int chars = 0;
		// maximum number of symbols that can be held in memory (used with -f option)
		int max_symbols = 0;

		S = new rules;

		size_t num = 0;
		int i = src[num++];

		min_terminal = max_terminal = i;

		S->last()->insert_after(new symbols(i));

		//
		// now loop reading characters (loop will end upon reaching end of input)
		//
		while (num < size) {
			i = src[num++];

			if (i < min_terminal) min_terminal = i;
			else if (i > max_terminal) max_terminal = i;

			// append read character to end of rule S, and enforce constraints
			S->last()->insert_after(new symbols(i));
			S->last()->prev()->check();

			// if memory limit reached, "forget" part of the grammar
			if (max_symbols && num_symbols > max_symbols)
				// if compression has not been initalized, initialize
				if (!compression_initialized) {
					start_compress(false); compression_initialized = true;
				}
				// send first symbol of (the remaining part of) the grammar
				// to the compressor
				forget(S->first());
		}

		// now all input has been read,
		// proceed to compression/printing out the grammar


		// initialize compression
		if (!compression_initialized) start_compress(true);

		void calculate_rule_usage(rules *r);
		if (print_rule_usage) calculate_rule_usage(S);

		// tell the compressor no more rules will be removed from memory
		stop_forgetting();
		while (S->first()->next() != S->first())
			// send symbol to the compressor
			forget(S->first());

		end_compress();

		if (do_print) {
			number();
			print();
		}

		return 0;
	}
private:
	rules *S;                 // pointer to main rule of the grammar

	int num_rules = 0;        // number of rules in the grammar
	int num_symbols = 0;      // number of symbols in the grammar
	int min_terminal,         // minimum and maximum value among terminal symbols
	max_terminal;         //
	int max_rule_len = 2;     // maximum rule length
	bool compression_initialized = false;

	int 	reproduce = 0,
			quiet = 0,
			phind = 0,
			numbers = 0,
			print_rule_freq = 0,
			print_rule_usage = 0,
			delimiter = -1,
			memory_to_use = 1000000000,

	// minimum number of times a digram must occur to form rule minus one
	// (e.g. if K is 1, two occurrences are required to form rule)
	K = 1;

	ofstream *rule_S = nullptr;

	const char *help = "\n\
usage: sequitur -cdpqrtTuz -k <K> -e <delimiter> -f <max symbols> -m <memory_limit>\n\n\
-p    print grammar at end\n\
-d    treat input as symbol numbers, one per line\n\
-c    compress\n\
-u    uncompress\n\
-m    use this amount of memory, in MB, for the hash table (default 1000)\n\
-q    quiet: suppress progress numbers on stderr\n\
-r    reproduce full expansion of rules after each rule\n\
-t    print rule usage in the grammar after each rule\n\
-T    print rule usage in the input after each rule\n\
-z    put rule S in file called S, other rules on stdout as usual\n\
-k    set K, the minmum number of times a digram must occur to form rule\n\
      (default 2)\n\
-e    set the delimiter symbol. Rules will not be formed across (i.e. \n\
      including) delimiters. If with -d, 0-9 are treated as numbers\n\
-f    set maximum symbols in grammar (memory limit). Grammar/compressed output\n\
      will be generated once the grammar reaches this size\n\
";
};

size_t sequitur_compress(const unsigned char *src, size_t size)
{

}


// print the rules out

rules **R1;
int Ri;

// print out the grammar (after non-terminals have been numbered)
void print()
{
  for (int i = 0; i < Ri; i ++) {
    cout << i << " -> ";
    for (symbols *p = R1[i]->first(); !p->is_guard(); p = p->next())
      if (p->non_terminal()) cout << p->rule()->index() << ' ';
      else cout << *p << ' ';
    if (i > 0 && print_rule_freq) cout << '\t' << R1[i]->freq();
    if (i > 0 && print_rule_usage) cout << "\t(" << R1[i]->usage() << ")";
    if (reproduce && i > 0) {
      cout << '\t';
      R1[i]->reproduce();
    }
    cout << endl;
  }

  if (print_rule_freq)
    cout << (num_symbols - Ri) << " symbols, " << Ri << " rules "
	 << (num_symbols * (sizeof(symbols) + 4) + Ri * sizeof(rules))
	 << " total space\n";

}

// number non-terminal symbols for printing
void number()
{
  R1 = (rules **) malloc(sizeof(rules *) * num_rules);
  memset(R1, 0, sizeof(rules *) * num_rules);
  R1[0] = S;
  Ri = 1;

  for (int i = 0; i < Ri; i ++)
    for (symbols *p = R1[i]->first(); !p->is_guard(); p = p->next())
      if (p->non_terminal() && R1[p->rule()->index()] != p->rule()) {
         p->rule()->index(Ri);
         R1[Ri ++] = p->rule();
      }
}

// **************************************************************************
// print out symbol of rule S - and, if it is a non-terminal, its rule's right hand -
// printing rule S to separate file
// **************************************************************************
void forget_print(symbols *s)
{
  // open file to write rule S to, if not already open
  if (rule_S == 0) rule_S = new ofstream("S");

  // symbol is non-terminal
  if (s->non_terminal()) {

    // remember the rule the symbol heads and delete the symbol
    rules *r = s->rule();
    delete s;

    // there are no more instances of this symbol in the grammar (rule contents has already been printed out),
    // so we print out the symbol and delete the rule
    if (r->freq() == 0) {
      *rule_S << r->index();
      // delete all symbols in the rule
      while (r->first()->next() != r->first())
	delete r->first();
      // delete rule itself
      delete r;
    }
    // there are still instances of this symbol in the grammar
    else {
      // print rule's right hand (to stdout) (this will change r->index())
      if (r->index() == 0)
	r->output();
      // print rule index, i.e. non-terminal symbol, to file for rule S
      *rule_S << r->index();
    }

  }
  // symbol is terminal (print it and delete it)
  else {
    *rule_S << *s;
    delete s;
  }

  // put space between symbols
  *rule_S << ' ';
}


void calculate_rule_usage(rules *r)
{
  for (symbols *p = r->first(); !p->is_guard(); p = p->next()) {
    if (p->non_terminal()) {
      p->rule()->usage(1);
      calculate_rule_usage(p->rule());
    }
  }
}
