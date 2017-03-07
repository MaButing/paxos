#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
	int list[] = {1,2,3,4,5,6,7};
	vector<int> v (list, list+7);
	cout<< find(v.begin(), v.end(), 6) - v.begin()<<endl;
	return 0;
}
