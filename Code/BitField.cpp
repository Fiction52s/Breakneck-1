#include "BitField.h"
#include <assert.h>

using namespace std;

BitField::BitField(int p_numOptions)
{
	
	numOptions = p_numOptions;
	numFields = numOptions / 32;
	assert(numFields > 0);
	if (numFields == 0)
	{
		int x = 5;
	}
	numFields = max(numFields, 1);
	optionField = new sf::Uint32[numFields];
	Reset();
}

BitField::~BitField()
{
	delete[] optionField;
}

void BitField::Set(const BitField & otherField)
{
	assert(numFields == otherField.numFields);
	/*if (numFields != otherField.numFields)figure
	{
		delete[] optionField;
		numFields = otherField.numFields;
		numOptions = otherField.numOptions;
		optionField = new sf::Uint32[numFields];
	}*/


	for (int i = 0; i < numFields; ++i)
	{
		optionField[i] = otherField.optionField[i];
	}
}

void BitField::SetBit(int index, bool val)
{
	int trueIndex = index % 32;
	int fieldIndex = index / 32;
	sf::Uint32 check = (1 << trueIndex);
	if (val)
	{
		optionField[fieldIndex] |= check;
	}
	else
	{
		optionField[fieldIndex] &= ~check;
	}
}

bool BitField::GetBit(int index) const
{
	int trueIndex = index % 32;
	int fieldIndex = index / 32;
	sf::Uint32 test = (1 << trueIndex);
	sf::Uint32 field = optionField[fieldIndex];
	if (field & test)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void BitField::Or(const BitField &b)
{
	assert(numFields == b.numFields);
	for (int i = 0; i < numFields; ++i)
	{
		optionField[i] |= b.optionField[i];
	}
}

void BitField::And(const BitField &b)
{
	assert(numFields == b.numFields);
	for (int i = 0; i < numFields; ++i)
	{
		optionField[i] &= b.optionField[i];
	}
}

bool BitField::Equals(BitField &otherField)
{
	assert(numFields == otherField.numFields);
	for (int i = 0; i < numFields; ++i)
	{
		if (optionField[i] != otherField.optionField[i])
			return false;
	}

	return true;
}

bool BitField::IsNonZero()
{
	for (int i = 0; i < numFields; ++i)
	{
		if (optionField[i] > 0)
		{
			return true;
		}
	}

	return false;
}



int BitField::GetOnCount( int startBit )
{
	int counter = 0;
	for (int i = startBit; i < numOptions; ++i)
	{
		if (GetBit(i))
			++counter;
	}

	return counter;
}

void BitField::Reset()
{
	for (int i = 0; i < numFields; ++i)
	{
		optionField[i] = 0;
	}
}

bool BitField::Load(std::istream &is)
{
	//Reset();
	for (int i = 0; i < numFields; ++i)
	{
		is >> optionField[i];
	}
	//change this to binary later
	//add error checking later

	return true;
}

void BitField::Save(std::ofstream &of)
{
	for (int i = 0; i < numFields; ++i)
	{
		of << optionField[i] << "\n";
	}
}

bool BitField::LoadBinary(std::istream &is)
{
	//is.read((char*)optionField, sizeof(sf::Uint32) * numFields);
	is.read((char*)optionField, sizeof(optionField[0]) * numFields);
	/*for (int i = 0; i < numFields; ++i)
	{
		is.read((char*)&(optionField[i]), sizeof(optionField[i]));
	}*/
	return true;
}

void BitField::SaveBinary(std::ofstream &of)
{
	//of.write((char*)optionField, sizeof(sf::Uint32) * numFields);

	of.write((char*)optionField, sizeof(optionField[0]) * numFields);
	/*for (int i = 0; i < numFields; ++i)
	{
		of.write((char*)&(optionField[i]), sizeof(optionField[i]));
	}*/
}

