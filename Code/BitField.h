#ifndef __BITFIELD_H__
#define __BITFIELD_H__

#include <fstream>
#include <SFML\Graphics.hpp>

struct BitField
{
	BitField(int p_numOptions);
	bool Load(std::ifstream &is);
	void Save(std::ofstream &of);
	~BitField();
	void SetBit(int index, bool val);
	bool GetBit(int index);
	void Or(const BitField &b);
	int GetOnCount();
	bool IsNonZero();
	void Reset();
	void Set(const BitField & otherField);
	sf::Uint32 *optionField;
	int numOptions;
	int numFields;
};

#endif