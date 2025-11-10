#ifndef CPET_140_FINAL_PROJECT_SYS_H
#define CPET_140_FINAL_PROJECT_SYS_H


class system {
public:
    // Which part of the current local date/time to fetch
    enum class PartDateTime {
        YEAR,
        MONTH,
        DAY,
        HOUR,
        MINUTE,
        SECOND
    };

    // Returns the requested part of the current local time.
    // Example: fetchTime(PartDateTime::HOUR) -> 0..23
    static int fetchTime(PartDateTime part);
};


#endif //CPET_140_FINAL_PROJECT_SYS_H