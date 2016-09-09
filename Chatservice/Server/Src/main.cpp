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




typedef Typelist<int, char, double> DgramImpl;
enum
{
    INTMEMBER = 0, CHARMEMBER = 1, DOUBLEMEMBER = 2
};
typedef DataContainer<DgramImpl> Datagram;

int main()
{
    Datagram temp(99, 'k', 23.45);
    Datagram temp2(temp);

    temp.Set<INTMEMBER>(1);
    temp.Set<CHARMEMBER>('a');


    std::cout << temp.Get<INTMEMBER>() << ' ' << temp.Get<CHARMEMBER>() << 
        ' ' << temp.Get<DOUBLEMEMBER>() << std::endl;


    std::cout << temp2.Get<INTMEMBER>() << ' ' << temp2.Get<CHARMEMBER>() <<
        ' ' << temp2.Get<DOUBLEMEMBER>() << std::endl;




	//auto v1 = MyFactory::CreateInt();
	//auto v2 = MyFactory::CreateChar();
	//auto v3 = MyFactory::CreateBool();

	//delete v1;
	//delete v2;
	//delete v3;

	//int arr[]{ 0,1,2 };

	//std::cout << arr[0] << arr[1] << arr[2];

	//std::for_each(std::begin(arr), std::end(arr), [](int& i) { i *= 2; });

	//std::cout << arr[0] << arr[1] << arr[2];

	//std::string s1 = "this is a long string this is a long string this is a long string this is a long string this is a long string this is a long string this is a long string this is a long string this is a long string";
	//std::string s2 = "s";
	//std::cout << "\nLong string size: " << sizeof(s1) << std::endl
	//	<< "Short string size: " << sizeof(s2) << std::endl;



	system("pause");
}