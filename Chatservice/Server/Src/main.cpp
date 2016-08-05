#include "WebClient.h"
#include <iterator>
#include <algorithm>

template<class...TArgs>
class Factory : public TArgs...
{ };

struct IntBuilder
{
	static int *CreateInt()
	{
		return new int(s_value);
	}
private:
	static const int s_value = 321;
};
struct CharBuilder
{
	static char *CreateChar()
	{
		return new char(s_value);
	}
private:
	static const char s_value = 'A';
};
struct BoolBuilder
{
	static bool *CreateBool()
	{
		return new bool(s_value);
	}
private:
	static const bool s_value = true;
};

typedef Factory<CharBuilder, IntBuilder, BoolBuilder> MyFactory;

int main()
{
	auto v1 = MyFactory::CreateInt();
	auto v2 = MyFactory::CreateChar();
	auto v3 = MyFactory::CreateBool();

	delete v1;
	delete v2;
	delete v3;

	int arr[]{ 0,1,2 };

	std::cout << arr[0] << arr[1] << arr[2];

	std::for_each(std::begin(arr), std::end(arr), [](int& i) { i *= 2; });

	std::cout << arr[0] << arr[1] << arr[2];

	std::string s1 = "this is a long string this is a long string this is a long string this is a long string this is a long string this is a long string this is a long string this is a long string this is a long string";
	std::string s2 = "s";
	std::cout << "\nLong string size: " << sizeof(s1) << std::endl
		<< "Short string size: " << sizeof(s2) << std::endl;


	system("pause");
}