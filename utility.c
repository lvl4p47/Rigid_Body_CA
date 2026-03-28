#include "utility.h"

int8_t dir_to_coords[8][2] = 
{
    {-1, -1},
    {0, -1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 1},
    {-1, 1},
    {-1, 0}
};

int min(int a, int b)
{
    if(a < b) return a;
    return b;
}

int max(int a, int b)
{
    if(a > b) return a;
    return b;
}

float power(int a, int n)
{
    float ans = 1;
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            ans *= a;
        }
    }
    if(n < 0)
    {
        for(int i = 0; i < -n; i++)
        {
            ans /= a;
        }
    }
    return ans;
}