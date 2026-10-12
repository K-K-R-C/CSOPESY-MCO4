#pragma once
#include <string>
#include <vector>

// Retro BIOS POST screen, then the CSOPESY splash. Both return true when done.
// SPACE skips the current stage (handy during development).
class BootSequence {
public:
    BootSequence();
    bool drawBios(float dt);
    bool drawSplash(float dt);

private:
    struct BiosLine { std::string text; bool ram; float at; };
    std::vector<BiosLine> lines_;
    std::string logo_[5];
    float t_ = 0.0f, biosEnd_ = 0.0f;
};