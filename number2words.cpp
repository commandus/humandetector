#include <sstream>
#include "number2words.h"

/**
 * @see http://algolist.ru/maths/misc/sumwrite.php
 */
#define DG_POWER 7              // Энто допустимая степень числа 1000 для int64_t:
                                
struct POWER {                
  int sex;                      
  const char *one;
  const char *four;
  const char *many;
} 
powerRU[] = {
  { 0, " "            ," "              ," "           },   // 1
  { 1, "тысяча "      ,"тысячи "       ,"тысяч "        },  // 2
  { 0, "миллион "     ,"миллиона "     ,"миллионов "    },  // 3
  { 0, "миллиард "    ,"миллиарда "    ,"миллиардов "   },  // 4
  { 0, "триллион "    ,"триллиона "    ,"триллионов "   },  // 5
  { 0, "квадриллион " ,"квадриллиона " ,"квадриллионов "},  // 6
  { 0, "квинтиллион " ,"квинтиллиона " ,"квинтиллионов "}   // 7
};

struct UNIT {
  const char *one[2];
  const char *two;
  const char *dec;
  const char *hun;
} 
digitsRU[10]= {
  { {""       ,""       },"десять "      ,""            ,""          },
  { {"один "  ,"одна "  },"одиннадцать " ,"десять "     ,"сто "      },
  { {"два "   ,"две "   },"двенадцать "  ,"двадцать "   ,"двести "   },
  { {"три "   ,"три "   },"тринадцать "  ,"тридцать "   ,"триста "   },
  { {"четыре ","четыре "},"четырнадцать ","сорок "      ,"четыреста "},
  { {"пять "  ,"пять "  },"пятнадцать "  ,"пятьдесят "  ,"пятьсот "  },
  { {"шесть " ,"шесть " },"шестнадцать " ,"шестьдесят " ,"шестьсот " },
  { {"семь "  ,"семь "  },"семнадцать "  ,"семьдесят "  ,"семьсот "  },
  { {"восемь ","восемь "},"восемнадцать ","восемьдесят ","восемьсот "},
  { {"девять ","девять "},"девятнадцать ","девяносто "  ,"девятьсот "}
};

const char *zero = "ноль ";

std::string number2stringRU(
	uint64_t value
)
{
	int i;
	uint64_t mny;
	std::stringstream result;
	uint64_t divisor;

	if (!value) {
		result << zero << powerRU[0].many;
		return result.str();
	}

	for (i = 0, divisor = 1; i < DG_POWER - 1; i++) {
		divisor *= 1000U;
	}
	for (i = DG_POWER - 1; i >= 0; i--) {
		mny = value / divisor;
		value %= divisor;
		if (mny == 0) {
			if (i > 0) {
				divisor /= 1000;
				continue;
			}
			result << powerRU[i].one;
		} else {
			if (mny >= 100) {
				result << digitsRU[mny / 100].hun;
				mny %= 100;
			}
			if (mny >= 20) {
				result << digitsRU[mny / 10].dec;
				mny %= 10;
			}
			if (mny >= 10)
				result << digitsRU[mny - 10].two;
			else if (mny >= 1)
				result << digitsRU[mny].one[powerRU[i].sex];
			switch (mny)
			{
			case 1:
				result << powerRU[i].one;
				break;
			case 2:
			case 3:
			case 4:
				result << powerRU[i].four;
				break;
			default:
				result << powerRU[i].many;
				break;
			};
		}
		divisor /= 1000;
	}
	return result.str();
}

struct POWER_CODE {
  int sex;                      
  int32_t one;
  int32_t four;
  int32_t many;
};

struct POWER_CODE powerCodeRU[] = {
  { 0, -3,        -3,        -3        },  // 1
  { 1, 1000,      1001,      1002      },  // 2
  { 0, 10000,     10001,     10002     },  // 3
  { 0, 100000,    100001,    100002    },  // 4
  { 0, 1000000,   1000001,   1000002   },  // 5
  { 0, 10000000,  10000001,  10000002  },  // 6
  { 0, 100000000, 100000001, 100000002 }   // 7
};

struct UNIT_CODE {
  int one[2];
  int two;
  int dec;
  int hun;
};

struct UNIT_CODE digitsCodeRU[10]= {
  { {-3, -3}, 10, -3, -3  },
  { {1, 188}, 11, 10, 100 },
  { {2, 288}, 12, 20, 200 },
  { {3, 3  }, 13, 30, 300 },
  { {4, 4  }, 14, 40, 400 },
  { {5, 5  }, 15, 50, 500 },
  { {6, 6  }, 16, 60, 600 },
  { {7, 7  }, 17, 70, 700 },
  { {8, 8  }, 18, 80, 800 },
  { {9, 9  }, 19, 90, 900 }
};

void number2codeRU (
	std::vector<int32_t> &retval,
	uint64_t value
) {
	int i;
	uint64_t mny;
	uint64_t divisor;

	if (!value) {
		retval.push_back(0);
		return;
	}

	for (i = 0, divisor = 1; i < DG_POWER - 1; i++) {
		divisor *= 1000U;
	}
	for (i = DG_POWER - 1; i >= 0; i--) {
		mny = value / divisor;
		value %= divisor;
		if (mny == 0) {
			if (i > 0) {
				divisor /= 1000;
				continue;
			}
			retval.push_back(powerCodeRU[i].one);
		} else {
			if (mny >= 100) {
				retval.push_back(digitsCodeRU[mny / 100].hun);
				mny %= 100;
			}
			if (mny >= 20) {
				retval.push_back(digitsCodeRU[mny / 10].dec);
				mny %= 10;
			}
			if (mny >= 10)
				retval.push_back(digitsCodeRU[mny - 10].two);
			else if (mny >= 1)
				retval.push_back(digitsCodeRU[mny].one[powerCodeRU[i].sex]);
			switch (mny)
			{
			case 1:
				retval.push_back(powerCodeRU[i].one);
				break;
			case 2:
			case 3:
			case 4:
				retval.push_back(powerCodeRU[i].four);
				break;
			default:
				retval.push_back(powerCodeRU[i].many);
				break;
			};
		}
		divisor /= 1000;
	}
}
