/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://www.qt.io/licensing.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

/*!
    \class QmlProfiler::SortedTimelineModel
    \brief The SortedTimelineModel class provides a sorted model for timeline data.

    The SortedTimelineModel lets you keep range data sorted by both start and end times, so that
    visible ranges can easily be computed. The only precondition for that to work is that the ranges
    must be perfectly nested. A "parent" range of a range R is defined as a range for which the
    start time is smaller than R's start time and the end time is greater than R's end time. A set
    of ranges is perfectly nested if all parent ranges of any given range have a common parent
    range. Mind that you can always make that happen by defining a range that spans the whole
    available time span. That, however, will make any code that uses firstStartTime() and
    lastEndTime() for selecting subsets of the model always select all of it.

    \note Indices returned from the various methods are only valid until a new range is inserted
          before them. Inserting a new range before a given index moves the range pointed to by the
          index by one. Incrementing the index by one will make it point to the item again.
*/

/*!
    \fn SortedTimelineModel::clear()
    Clears the ranges and their end times.
*/

/*!
    \fn int SortedTimelineModel::count() const
    Returns the number of ranges in the model.
*/

/*!
    \fn qint64 SortedTimelineModel::firstStartTime() const
    Returns the begin of the first range in the model.
*/

/*!
    \fn qint64 SortedTimelineModel::lastEndTime() const
    Returns the end of the last range in the model.
*/

/*!
    \fn const SortedTimelineModel<Data>::Range &SortedTimelineModel::range(int index) const
    Returns the range data at the specified index.
*/

/*!
    \fn Data &SortedTimelineModel::data(int index)
    Returns modifiable user data for the range at the specified index.
*/

/*!
    \fn int SortedTimelineModel::insert(qint64 startTime, qint64 duration, const Data &item)
    Inserts the given data at the given time position and returns its index.
*/

/*!
    \fn int SortedTimelineModel::insertStart(qint64 startTime, const Data &item)
    Inserts the given data as range start at the given time position and
    returns its index. The range end is not set.
*/

/*!
    \fn int SortedTimelineModel::insertEnd(int index, qint64 duration)
    Adds a range end for the given start index.
*/

/*!
    \fn int SortedTimelineModel::findFirstIndexNoParents(qint64 startTime) const
    Looks up the first range with an end time greater than the given time and
    returns its index. If no such range is found, it returns -1.
*/

/*!
    \fn int SortedTimelineModel::findFirstIndex(qint64 startTime) const
    Looks up the first range with an end time greater than the given time and
    returns its parent's index. If no such range is found, it returns -1. If there
    is no parent, it returns the found range's index. The parent of a range is the
    range with the lowest start time that completely covers the child range.
    "Completely covers" means:
    parent.startTime <= child.startTime && parent.endTime >= child.endTime
*/

/*!
    \fn int SortedTimelineModel::findLastIndex(qint64 endTime) const
    Looks up the last range with a start time smaller than the given time and
    returns its index. If no such range is found, it returns -1.
*/

/*!
    \fn void computeNesting()
    Compute all ranges' parents.
    \sa findFirstIndex
*/
