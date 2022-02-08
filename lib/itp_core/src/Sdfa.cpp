#include "Sdfa.h"

namespace itp
{

bool less(long lhs, size_t rhs)
{
	return (lhs < 0) || (static_cast<size_t>(lhs) < rhs);
}

SensingDFA::SensingDFA(Symbol min_symbol, Symbol max_symbol)
	: MultiheadAutomaton<10>{min_symbol, max_symbol}
	, h3a_{h(0)}
	, inner_{h(5)}
	, outer_{h(6)}
	, l_{h(7)}
	, r_{h(8)}
	, t_{h(9)}
{
	SetHeadName(0, "h3a");
	SetHeadName(1, "h1");
	SetHeadName(2, "h2");
	SetHeadName(3, "h3");
	SetHeadName(4, "h4");
	SetHeadName(5, "inner");
	SetHeadName(6, "outer");
	SetHeadName(7, "l");
	SetHeadName(8, "r");
	SetHeadName(9, "t");
}

void SensingDFA::Run()
{
	while (less(h(4), a.size()))
	{
		GuessIfRightmost(r_, IsPredictionConfident::No);
		if (!Move(r_) || !Correction() || !Matching())
		{
			return;
		}
	}
}

Symbol SensingDFA::MeanSymbol() const
{
	return static_cast<Symbol>((static_cast<size_t>(GetMaxSymbol()) + static_cast<size_t>(GetMinSymbol())) / 2);
}

void SensingDFA::GuessIfRightmost(const Head& h, IsPredictionConfident confidence)
{
	GuessIfRightmost(h, a[h], confidence);
}

void SensingDFA::GuessIfRightmost(const Head& h, Symbol predicted_symbol, IsPredictionConfident confidence)
{
	if (IsRightmost(h))
	{
		if (confidence == IsPredictionConfident::Yes)
		{
			Guess(predicted_symbol, confidence);
		}
		else
		{
			Guess(MeanSymbol(), confidence);
		}
	}
}

bool SensingDFA::AdvanceOne(size_t i)
{
	while (t_ != h(i))
	{
		Move(t_);
	}

	GuessIfRightmost(h(i), IsPredictionConfident::No);
	EXIT_IF_IMPOSSIBLE(Move(h(i)));

	while (inner_ != r_)
	{
		Move(inner_);
	}

	while (l_ != inner_)
	{
		if (a[t_] == a[h(i)])
		{
			EXIT_IF_IMPOSSIBLE(Move(l_, r_, outer_));
		}
		else
		{
			while (inner_ != r_)
			{
				Move(inner_);
			}

			GuessIfRightmost(h(i), IsPredictionConfident::No);
			EXIT_IF_IMPOSSIBLE(Move(h(i)))
		}

		EXIT_IF_IMPOSSIBLE(Move(t_));
		GuessIfRightmost(h(i), IsPredictionConfident::No);
		EXIT_IF_IMPOSSIBLE(Move(h(i)));
	}

	while (a[t_] == a[h(i)])
	{
		EXIT_IF_IMPOSSIBLE(Move(t_));
		GuessIfRightmost(h(i), a[t_], IsPredictionConfident::Yes);
		EXIT_IF_IMPOSSIBLE(Move(h(i)));
	}

	return true;
}

bool SensingDFA::AdvanceMany(size_t i)
{
	while (outer_ != r_)
	{
		Move(outer_);
	}
	while (l_ != outer_)
	{
		EXIT_IF_IMPOSSIBLE(AdvanceOne(i) && Move(l_, r_));
	}

	return true;
}

bool SensingDFA::Correction()
{
	while (h(1) != h(4))
	{
		Move(h(1));
	}
	EXIT_IF_IMPOSSIBLE(AdvanceOne(1));

	while (h(2) != h(1))
	{
		Move(h(2));
	}
	EXIT_IF_IMPOSSIBLE(AdvanceMany(2));

	while (h(3) != h(2))
	{
		Move(h(3));
	}
	EXIT_IF_IMPOSSIBLE(AdvanceMany(3));

	while (h(4) != h(3))
	{
		Move(h(4));
	}
	EXIT_IF_IMPOSSIBLE(AdvanceMany(4));

	return true;
}

bool SensingDFA::Matching()
{
	while (less(h(4), a.size()))
	{
		while (h3a_ != h(3))
		{
			Move(h3a_);
		}

		while ((a[h(1)] == a[h(2)]) && (a[h(2)] == a[h(3)]) && (a[h(3)] == a[h(4)]))
		{
			EXIT_IF_IMPOSSIBLE(Move(h(1), h(2), h3a_, h(3)));
			GuessIfRightmost(h(4), a[h(2)], IsPredictionConfident::Yes);
			EXIT_IF_IMPOSSIBLE(Move(h(4)));
		}

		if (a[h(2)] != a[h(4)])
		{
			break;
		}

		while ((a[h(2)] == a[h(3)]) && (a[h(3)] == a[h(4)]))
		{
			EXIT_IF_IMPOSSIBLE(Move(h(2), h(3)));
			GuessIfRightmost(h(4), a[h(3)], IsPredictionConfident::Yes);
			EXIT_IF_IMPOSSIBLE(Move(h(4)));
		}

		if (a[h(3)] != a[h(4)])
		{
			break;
		}

		while ((a[h3a_] == a[h(3)]) && (a[h(3)] == a[h(4)]))
		{
			EXIT_IF_IMPOSSIBLE(Move(h3a_, h(3)));
			GuessIfRightmost(h(4), a[h3a_], IsPredictionConfident::Yes);
			EXIT_IF_IMPOSSIBLE(Move(h(4)));
		}

		if (a[h3a_] != a[h(4)])
		{
			break;
		}

		while ((h3a_ != h(3)) && (a[h3a_] == a[h(4)]))
		{
			EXIT_IF_IMPOSSIBLE(Move(h3a_));
			GuessIfRightmost(h(4), a[h3a_], IsPredictionConfident::Yes);
			EXIT_IF_IMPOSSIBLE(Move(h(4)));
		}

		if (a[h3a_] != a[h(4)])
		{
			break;
		}
	}

	return true;
}

} // namespace itp
