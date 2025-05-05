#pragma once

class IBacteria {
    public:
        virtual void update(float dt) = 0;
        virtual bool canDivide() const = 0;
        virtual void applyAntibiotic(float intensity) = 0;
        virtual IBacteria* clone() const = 0;
        virtual ~IBacteria() = default;
};

