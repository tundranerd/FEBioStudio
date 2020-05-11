#include "stdafx.h"
#include "units.h"

#ifdef _DEBUG
static int unit_system = Units::SI;
#else
static int unit_system = Units::NONE;
#endif

void Units::SetUnitSystem(unsigned int us) { unit_system = us; }
unsigned int Units::GetUnitSystem() { return unit_system; }

QStringList Units::SupportedUnitSystems()
{
	QStringList s;
	s << "None" << "Dimensions only" << "SI" << "MMTS" << "CGS";

	return s;
}

static const char* unit_table[][10] = {
	// dimensions
	{"L","M","t","T","l","n","F","P","E","W"},

	// SI units
	{"m","kg","s","K","A","mol","N","Pa","J","W"},

	// MMTS units
	{"mm","ton","s","K","A","mol","N","MPa","mJ","mW"},

	// CGS units
	{"cm","g","s","K","A","mol","dyn","Ba","erg","erg/s"}
};

QString Units::unitSymbol(int us, Unit_Symbol sym)
{
	if (us == NONE) return "";
	return unit_table[us - 1][sym];
}

// prefixes
#define GIGA  "G"	// 10^9
#define MEGA  "M"	// 10^6
#define KILO  "k"	// 10^3
#define HECTO "h"	// 10^2
#define DECA  "da"	// 10^1
#define DECI  "d"	// 10^-1
#define CENTI "c"	// 10^-2
#define MILLI "m"	// 10^-3
#define MICRO "mu"	// 10^-6
#define NANO  "n"	// 10^-9

// turn this off to use non-unicode representation of symbols
#define USE_UNICODE

#ifdef USE_UNICODE
#define POW_2 QString(QChar(0x00B2))
#define POW_3 QString(QChar(0x00B3))
#define POW_4 QString(QChar(0x2074))
#define DEG QString(QChar(0x00B0))
#else
#define POW_2 QString("^2")
#define POW_3 QString("^3")
#define POW_4 QString("^4")
#define DEG QString("deg")
#endif

#define L u[LENGTH]
#define M u[MASS]
#define t u[TIME]
#define T u[TEMPERATURE]
#define l u[CURRENT]
#define n u[SUBSTANCE]
#define F u[FORCE]
#define P u[PRESSURE]
#define E u[ENERGY]
#define W u[POWER]
#define RAD QString("rad")

QString Units::GetUnitString(const char* szunit)
{
	if (szunit == 0) return "";
	if (unit_system == UNIT_SYSTEM::NONE) return "";

	const char* (*u) = unit_table[unit_system - 1];

	// parse unit string
	QString s;
	const char* c = szunit;
	while (*c)
	{
		switch (*c)
		{
		case 'L': s += L; break;
		case 'M': s += M; break;
		case 't': s += t; break;
		case 'T': s += T; break;
		case 'l': s += l; break;
		case 'n': s += n; break;
		case 'F': s += F; break;
		case 'P': s += P; break;
		case 'E': s += E; break;
		case 'W': s += W; break;
		case 'd': s += DEG; break;
		case 'r': s += RAD; break;
		case '/': s += '/'; break;
		case '.': s += '.'; break;
		case '^':
		{
			c++;
			if (*c == 0) { assert(false); return "?"; }
			int i = *c - '0';
			switch (i)
			{
			case 2: s += POW_2; break;
			case 3: s += POW_3; break;
			case 4: s += POW_4; break;
			default: 
				s += QString("^%1").arg(i);
			}
		}
		break;
		default:
			assert(false);
		}

		c++;
	}

	if (unit_system == Units::DIMENSIONAL) s = QString("[") + s + QString("]");

	return s;
}
