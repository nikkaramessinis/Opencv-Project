#pragma once
class Fruit
{
public:
	int GetXPos() const 
	{
		return x;
	}

	int GetYPos() const
	{
		return y;
	}

	void SetXPos(int _x) 
	{
		x = _x;
	}

	void SetYPos(int _y)
	{
		y = _y;
	}
private:
	int x; 
	int y;
};

