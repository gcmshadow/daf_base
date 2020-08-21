// -*- lsst-c++ -*-

/*
 * LSST Data Management System
 * Copyright 2008, 2009, 2010 LSST Corporation.
 *
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program.  If not,
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */

#ifndef LSST_DAF_BASE_DATETIME_H
#define LSST_DAF_BASE_DATETIME_H

/** @file
 * @ingroup daf_base
 *
 * @brief Interface for DateTime class
 *
 * @author Kian-Tat Lim (ktl@slac.stanford.edu)
 * @version $Revision$
 * @date $Date$
 */

/** @class lsst::daf::base::DateTime
 * @brief Class for handling dates/times, including MJD, UTC, and TAI.
 *
 * @ingroup daf_base
 */

#include <cstdint>
#include <ctime>
#include <limits>
#include <sys/time.h>
#include <string>

#include "lsst/base.h"
#include "lsst/pex/exceptions.h"

// Forward declaration of the boost::serialization::access class.
namespace boost {
namespace serialization {
class access;
}
}  // namespace boost

namespace lsst {
namespace daf {
namespace base {

class LSST_EXPORT DateTime {
public:
    enum DateSystem { JD = 0, MJD, EPOCH };  // EPOCH is Julian epoch year
        // e.g. 2000.0 for J2000
    enum Timescale {
        TAI = 5,
        UTC,
        TT
    };  // use values that do not overlap DateSystem
        // to avoid confusing one for the other in Python
    // an invalid DateTime has _nsec == invalid_nsecs
    constexpr static long long invalid_nsecs = std::numeric_limits<std::int64_t>::min();

    /**
     * Default constructor: construct an invalid DateTime
     */
    explicit DateTime();

    /**
     * Construct a DateTime from nanoseconds since the unix epoch
     *
     * @param[in] nsecs  integer nanoseconds since the unix epoch;
                         if nsecs == DateTime.invalid_nsecs then the DateTime is invalid,
                         regardless of scale
     * @param[in] scale  time scale of input (TAI, TT or UTC, default TAI).
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the date is before 1961-01-01
     */
    explicit DateTime(long long nsecs, Timescale scale = TAI);

    /**
     * Construct a DateTime from a double in the specified system and scale
     *
     * @param[in] date  specified date
     * @param[in] system  time system of input (JD, MJD, or [Julian] EPOCH)
     * @param[in] scale  time scale of input (TAI, TT or UTC, default TAI).
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the date is before 1961-01-01
     */
    explicit DateTime(double date, DateSystem system = MJD, Timescale scale = TAI);

    /**
     * Construct a DateTime from year, month, day, etc. (tm struct fields)
     *
     * @param[in] year   year; must be in the range [1902, 2261], inclusive.
     * @param[in] month  month number, where January = 1
     * @param[in] day  day of the month (1 to 31).
     * @param[in] hr  hour (0 to 23)
     * @param[in] min  minute (0 to 59)
     * @param[in] sec  integer seconds (0 to 60)
     * @param[in] scale  time scale of input (TAI, TT or UTC, default TAI).
     * @throw lsst.pex.exceptions.DomainError if `year` is < 1902 or > 2261,
     * or if `scale` is UTC and the date is before 1961-01-01 (the start of the leap second table)
     */
    DateTime(int year, int month, int day, int hr, int min, int sec, Timescale scale = TAI);

    /**
     * Construct a DateTime from an ISO8601 string
     *
     * Accepts a restricted subset of ISO8601:
     *     yyyy-mm-ddThh:mm:ss.nnnnnnnnnZ
     * where:
     * - the final Z is required for UTC and prohibited for TAI or TT
     * - the - and : separators are optional
     * - the decimal point and fractional seconds are optional
     * - the decimal point may be a comma
     *
     * @param[in] iso8601  ISO8601 string representation of date and time
     * @param[in] scale  time scale of input (TAI, TT or UTC, default TAI).
     * @throw lsst.pex.exceptions.DomainError if `year` is < 1902 or > 2261,
     * or if `scale` is UTC and the date is before 1961-01-01 (the start of the leap second table)
     */
    explicit DateTime(std::string const& iso8601, Timescale scale);

    /**
     * Get date as nanoseconds since the unix epoch
     *
     * @note If the DateTime is invalid then the returned value is DateTime.invalid_nsecs,
     * regardless of scale.
     *
     * @param[in] scale  desired time scale (TAI, TT or UTC)
     * @return the date as nanoseconds since the unix epoch in the specified time scale
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     */
    long long nsecs(Timescale scale = TAI) const;

    /**
     * Get date as a double in a specified representation, such as MJD
     *
     * @return the date in the required system, for the requested scale
     * @param[in] system  desired time system (JD, MJD, or [Julian] EPOCH)
     * @param[in] scale  desired time scale (TAI, TT or UTC)
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     * @throw lsst.pex.exceptions.RuntimeError if DateTime is invalid
     */
    double get(DateSystem system = MJD, Timescale scale = TAI) const;

    /** Get date as an ISO8601-formatted string.
     *
     * The returned format is:
     *    yyyy-mm-ddThh:mm:ss.sssssssssZ
     * where the final Z is only present if scale is UTC
     *
     * @param[in] scale  Desired time scale (TAI, TT or UTC).
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     * @throw lsst.pex.exceptions.RuntimeError if DateTime is invalid
     */
    std::string toString(Timescale scale) const;

    /** Get date as a tm struct, with truncated fractional seconds
     *
     * @param[in] scale  desired time scale (TAI, TT or UTC)
     * @return date as a tm struct
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     * @throw lsst.pex.exceptions.RuntimeError if DateTime is invalid
     */
    struct tm gmtime(Timescale scale) const;

    /**
     * Get date as a timespec struct, with time in seconds and nanoseconds
     *
     * @param[in] scale  Desired time scale (TAI, TT or UTC)
     * @return date as a timespec struct
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     * @throw lsst.pex.exceptions.RuntimeError if DateTime is invalid
     */
    struct timespec timespec(Timescale scale) const;

    /**
     * Get date as a timeval struct, with time in seconds and microseconds
     *
     * @param[in] scale  desired time scale (TAI, TT or UTC)
     * @return date as a timeval struct
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     * @throw lsst.pex.exceptions.RuntimeError if DateTime is invalid
     */
    struct timeval timeval(Timescale scale) const;

    /**
     * Is this date valid?
     */
    bool isValid() const { return _nsecs != DateTime::invalid_nsecs; };

    bool operator==(DateTime const& rhs) const;

    /// Return a hash of this object.
    std::size_t hash_value() const noexcept;

    /** Return current time as a DateTime
     *
     * Assumes the system clock keeps UTC, as almost all computers do.
     */
    static DateTime now(void);

    /**
     * Initialize the leap second table from USNO
     *
     * The data can be found here: http://maia.usno.navy.mil/ser7/tai-utc.dat
     * and is saved in DateTime.cc as static constant `leapString`.
     *
     * @param leapString  leap second table from USNO as a single multiline string.
     */
    static void initializeLeapSeconds(std::string const& leapString);

private:
    long long _nsecs;  ///< TAI nanoseconds since Unix epoch

    /// Raise RuntimeError if DateTime is not valid
    void _assertValid() const {
        if (!isValid()) {
            throw LSST_EXCEPT(pex::exceptions::RuntimeError, "DateTime not valid");
        }
    }

    /**
     * Get date as Modified Julian Days in the specified time scale
     *
     * @param[in] scale  time scale (TAI, TT or UTC)
     * @return date as Modified Julian Days
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     */
    double _getMjd(Timescale scale) const;

    /**
     * Get date as Julian Days in the specified time scale
     *
     * @param[in] scale  time scale (TAI, TT or UTC)
     * @return date as Julian Days
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     */
    double _getJd(Timescale scale) const;

    /**
     * Get date as a Julian epoch year in the specified time scale
     * (e.g. J2000 is 2000.0)
     *
     * @param[in] scale  time scale (TAI, TT or UTC)
     * @return date as a Julian epoch year
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     */
    double _getEpoch(Timescale scale) const;

    /**
     * Set internal nanoseconds from Modified Julian Date in the specified time scale
     *
     * @param[in] mjd  date as a Modified Julian Day
     * @param[in] scale  time scale of input (TAI, TT or UTC)
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     */
    void setNsecsFromMjd(double mjd, Timescale scale);

    /**
     * Set internal nanoseconds from Julian Days in the specified time scale
     *
     * @param[in] jd  date as a Julian Day
     * @param[in] scale  time scale of date (TAI, TT or UTC)
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     */
    void setNsecsFromJd(double jd, Timescale scale);

    /**
     * Set internal nanoseconds from a Julian epoch year in the specified time scale
     *
     * @param[in] epoch  date as a Julian epoch year, e.g. 2000.0 for J2000
     * @param[in] scale  time scale of date (TAI, TT or UTC)
     * @throw lsst.pex.exceptions.DomainError if scale is UTC and the UTC date is before 1961-01-01
     */
    void setNsecsFromEpoch(double epoch, Timescale scale);

    friend class boost::serialization::access;

    /**
     * Serialize DateTime to/from a Boost archive
     *
     * @param[in,out] ar   archive to access
     * @param[in] version  version of class serializer
     */
    template <class Archive>
    void serialize(Archive ar, int const version) {
        ar& _nsecs;
    }
};

}  // namespace base
}  // namespace daf
}  // namespace lsst

namespace std {
template <>
struct hash<lsst::daf::base::DateTime> {
    using argument_type = lsst::daf::base::DateTime;
    using result_type = size_t;
    size_t operator()(argument_type const& obj) const noexcept { return obj.hash_value(); }
};
}  // namespace std

#endif
