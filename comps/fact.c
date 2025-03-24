long long int fact(long long int n) {
    if (n > 0) {
        return n * fact(n - 1);
    } else {
        return 1;
    }
}