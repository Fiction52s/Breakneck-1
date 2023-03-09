#ifndef __BITFIELD_H__
#define __BITFIELD_H__

#include <fstream>
#include <SFML\Graphics.hpp>

struct BitField
{
	sf::Uint32 *optionField;
	int numOptions;
	int numFields;

	BitField(int p_numOptions);
	bool Load(std::istream &is);
	void Save(std::ofstream &of);
	bool LoadBinary(std::istream &is);
	void SaveBinary(std::ofstream &of);
	~BitField();
	void SetBit(int index, bool val);
	bool GetBit(int index);
	void Or(const BitField &b);
	void And(const BitField &b);
	int GetOnCount( int startBit = 0);
	bool IsNonZero();
	bool Equals(BitField &otherField);
	void Reset();
	void Set(const BitField & otherField);
};

#endif