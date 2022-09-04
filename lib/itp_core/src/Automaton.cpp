#include "Automaton.h"

#include "ItpExceptions.h"

namespace itp
{

AutomatonWord::AutomatonWord(const PlainTimeSeries<Symbol>& time_series)
	: time_series_(begin(time_series), end(time_series))
{
	// DO NOTHING
}

ssize_t AutomatonWord::size() const
{
	return time_series_.size();
}

const AutomatonWord::Ext_symbol_t& AutomatonWord::operator[](ssize_t n) const
{
	if (0 <= n)
	{
		return time_series_[n];
	}

	if (n != -1)
	{
		throw RangeError(
			std::string("AutomatonWord: negative index ") + std::to_string(n) + std::string(", only -1 is allowed."));
	}

	return kBeginningDelimiter;
}

void PredictionAutomaton::SetMinSymbol(Symbol)
{
	// DO NOTHING
}

void PredictionAutomaton::SetMaxSymbol(Symbol)
{
	// DO NOTHING
}

HighPrecDouble KrichevskyPredictor(Symbol sym, size_t sym_freq, size_t total_freq, size_t alphabet_size)
{
	return (HighPrecDouble(sym_freq) + 1. / 2) / (HighPrecDouble(total_freq) + alphabet_size / 2.);
}

} // namespace itp
