#pragma once

namespace disyn::hal {

class Gate {
public:
    void begin(int pinIn, int pinOut);
    bool read() const;
    void write(bool high);

private:
    int pinIn_ = -1;
    int pinOut_ = -1;
};

} // namespace disyn::hal
