#ifndef MYFUNCTION_H
#define MYFUNCTION_H


int add(int i, int j) {
    return i + j;
}

static float minusBy5(float i) {
    return i - 5.0f;
}
static float minusBy6(float i, int j, int k) {
    return i + j + k;
}


class MyFunction {
public:
    std::string concat(std::string& str) const {
        return info_ + str;
    }

    static int multiply(int i, int j) {
        return i * j;
    }

private:
    std::string info_ = "MyFunction : ";
};

#endif