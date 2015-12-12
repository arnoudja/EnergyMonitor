
#ifndef VALUERANGE_H
#define	VALUERANGE_H

#include <sstream>
#include <iomanip>

template <class T>
class ValueRange
{
public:
    ValueRange(T initialValue)
    {
        myCurrentValue = initialValue;
        myMinValue     = initialValue;
        myMaxValue     = initialValue;
        myRecordValue  = initialValue;
    }

    virtual ~ValueRange()       {}

    void updateValue(T value)
    {
        myCurrentValue = value;

        if (value < myMinValue)
        {
            myMinValue = value;
        }

        if (value > myMaxValue)
        {
            myMaxValue = value;
        }
        
        if (value > myRecordValue)
        {
            myRecordValue = value;
        }
    }

    void newPeriod()
    {
        myMinValue = myCurrentValue;
        myMaxValue = myCurrentValue;
    }

    T getCurrentValue() const   { return myCurrentValue; }
    T getMinValue() const       { return myMinValue; }
    T getMaxValue() const       { return myMaxValue; }
    T getRecordValue() const    { return myRecordValue; }

    std::string getRange() const
    {
        std::stringstream result;

        result << std::setprecision(3) << std::fixed;
        result << myMinValue << "-" << myMaxValue;

        return result.str();
    }

private:
    T myCurrentValue;
    T myMinValue;
    T myMaxValue;
    T myRecordValue;
};

#endif	/* VALUERANGE_H */
