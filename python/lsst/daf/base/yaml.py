# This file is part of daf_base
#
# Developed for the LSST Data Management System.
# This product includes software developed by the LSST Project
# (http://www.lsst.org/).
# See the COPYRIGHT file at the top-level directory of this distribution
# for details of code ownership.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""This module defines YAML I/O for key lsst.daf.base classes."""

# If yaml package is not installed there is no reason to fail everywhere
try:
    import yaml
except ImportError:
    yaml = None

from .dateTime import DateTime
from .propertyContainer import PropertyList, getPropertyListState, setPropertyListState, \
    getPropertySetState, setPropertySetState, PropertySet

# For YAML >= 5.1 need a different Loader for the constructor
loaderList = []
if yaml:
    loaderList = [yaml.Loader, yaml.CLoader]
    try:
        loaderList.append(yaml.FullLoader)
    except AttributeError:
        pass
    try:
        loaderList.append(yaml.UnsafeLoader)
    except AttributeError:
        pass
    try:
        loaderList.append(yaml.SafeLoader)
    except AttributeError:
        pass


# YAML representers for key lsst.daf.base classes


def dt_representer(dumper, data):
    """Represent an lsst.daf.base.DateTime (as ISO8601-formatted string in TAI)
    """
    return dumper.represent_scalar('lsst.daf.base.DateTime',
                                   data.toString(DateTime.TAI))


if yaml:
    yaml.add_representer(DateTime, dt_representer)


def pl_representer(dumper, data):
    """Represent an lsst.daf.base.PropertyList as an ordered sequence of
    name/type/value/comment lists)
    """
    # Turn the tuples into lists for cleaner representation in yaml
    result = getPropertyListState(data, asLists=True)
    return dumper.represent_sequence('lsst.daf.base.PropertyList', result,
                                     flow_style=None)


if yaml:
    yaml.add_representer(PropertyList, pl_representer)


def ps_representer(dumper, data):
    """Represent an lsst.daf.base.PropertySet as a mapping from names to
    type/value pairs.
    """
    # Turn the tuples into lists for cleaner representation in yaml
    result = getPropertySetState(data, asLists=True)
    return dumper.represent_sequence('lsst.daf.base.PropertySet', result,
                                     flow_style=None)


if yaml:
    yaml.add_representer(PropertySet, ps_representer)

###############################################################################

# YAML constructors for key lsst.daf.base classes


def dt_constructor(loader, node):
    """Construct an lsst.daf.base.DateTime from an ISO8601-formatted string in
    TAI
    """
    dt = loader.construct_scalar(node)
    return DateTime(str(dt), DateTime.TAI)


def pl_constructor(loader, node):
    """Construct an lsst.daf.base.PropertyList from a YAML pickle-like
    structure.
    """
    pl = PropertyList()
    yield pl
    state = loader.construct_sequence(node, deep=True)
    setPropertyListState(pl, state)


def ps_constructor(loader, node):
    """Construct an lsst.daf.base.PropertyList from a YAML pickle-like
    structure.
    """
    ps = PropertySet()
    yield ps
    state = loader.construct_sequence(node, deep=True)
    setPropertySetState(ps, state)


if yaml:
    for loader in loaderList:
        yaml.add_constructor('lsst.daf.base.PropertyList', pl_constructor, Loader=loader)
        yaml.add_constructor('lsst.daf.base.PropertySet', ps_constructor, Loader=loader)
        yaml.add_constructor('lsst.daf.base.DateTime', dt_constructor, Loader=loader)
