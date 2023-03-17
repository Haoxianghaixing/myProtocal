#include <iostream>

using namespace std;

int gcd(long long int a, long long int b) {
    if (b == 0) {
        return a;
    }
    printf("%lld %lld\n", a, a % b );
    return gcd(b, a % b);
}

long long lcm(long long a, long long b) {
    long long g = gcd(a, b);
    cout << g << endl;
    return (a / g) * b;
}

int main() {
    long long a = 19854681;
    long long b = 4970210;
    long long result = gcd(a, b);
    cout << "lcm(" << a << ", " << b << ") = " << result << endl;
    return 0;
}
