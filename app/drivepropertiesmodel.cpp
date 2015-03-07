/****************************************************************************
 * DisKMonitor, KDE tools to monitor SMART and MDRaid health status         *
 * Copyright (C) 2014-2015 Michaël Lhomme <papylhomme@gmail.com>            *
 *                                                                          *
 * This program is free software; you can redistribute it and/or modify     *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation; either version 2 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * This program is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License along  *
 * with this program; if not, write to the Free Software Foundation, Inc.,  *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.              *
 ****************************************************************************/


#include "drivepropertiesmodel.h"

#include "diskmonitor_settings.h"

#include <KLocalizedString>

#include <QFont>
#include <QBrush>
#include <QColor>
#include <QPalette>

#include "humanize.h"


/*
 * Constructor
 */
DrivePropertiesModel::DrivePropertiesModel()
{
  sensitiveAttributes = DiskMonitorSettings::sensitiveAttributes();
  headerLabels << i18nc("Attribute's id", "Id")
               << i18nc("Attribute's name", "Name")
               << i18nc("Attribute's flags", "Flags")
               << i18nc("Worst known value for this attribute", "Worst")
               << i18nc("Threshold value for this attribute", "Threshold")
               << i18nc("Normalized value for this attribute", "Normalized value")
               << i18nc("Human friendly value for this attribute", "Value");

  connect(DiskMonitorSettings::self(), SIGNAL(configChanged()), this, SLOT(configChanged()));
}



/*
 * Destructor
 */
DrivePropertiesModel::~DrivePropertiesModel()
{
}



/*
 * Retrieve the Drive associated to the model. Can be NULL
 */
Drive* DrivePropertiesModel::getDrive() const
{
  return (Drive*) this -> unit;
}



/*
 * Handler when StorageUnitPropertiesModel refresh the underlying unit
 */
void DrivePropertiesModel::updateInternalState()
{
  Drive* drive = getDrive();

  if(drive != NULL)
    attributes = drive -> getSMARTAttributes();
  else
    attributes.clear();
}



/*
 * Get the number of rows contained in the model's data
 */
int DrivePropertiesModel::rowCount(const QModelIndex& /*index*/) const
{
  return attributes.size();
}



/*
 * Get the number of column of the model
 */
int DrivePropertiesModel::columnCount(const QModelIndex& /*index*/) const
{
  return headerLabels.size();
}



/*
 * Retrieve data for an item in the model
 */
QVariant DrivePropertiesModel::data(const QModelIndex& index, int role) const
{
  if(!index.isValid() || unit == NULL)
    return QVariant();

  SmartAttribute attr = attributes.at(index.row());

  // Handle background colors
  if(role == Qt::BackgroundRole) {
    //value is unknown
    if(attr.value == -1)
      return QVariant(QBrush());

    switch(attr.healthStatus) {
      //set the row background to 'error' if attr has failing flag
      case HealthStatus::Failing:
        return QVariant(QBrush(DiskMonitorSettings::errorColor()));
        break;

      //set the row background to 'warning' if attr has warning flag
      case HealthStatus::Warning:
        return QVariant(QBrush(DiskMonitorSettings::warningColor()));
        break;

      default:
        return QVariant();
        break;
    }


  // Handle text colors
  } else if(role == Qt::ForegroundRole) {

    if(attr.value == -1) {
      return QVariant(QPalette().brush(QPalette::Disabled, QPalette::Text));
    } else
      return QVariant(QBrush());

  // Handle display value
  } else if(role == Qt::DisplayRole) {
    switch(index.column()) {
    case 0: return QVariant(attr.id); break;
    case 1: return QVariant(attr.name); break;
    case 2: return QVariant(attr.flags); break;
    case 3: return QVariant(attr.worst); break;
    case 4: return QVariant(attr.threshold); break;
    case 5: return QVariant(attr.value); break;
    case 6: return humanizeSmartAttribute(attr); break;
    //case 7: return QVariant(attr.pretty_unit); break;
    default: return QVariant(); break;
    }


  // Handle tooltips
  } else if(index.column() == 6 && role == Qt::ToolTipRole)
    return QVariant(i18n("Raw value: %1", QString::number(attr.pretty)));

  return QVariant();
}



/*
 * Format the 'pretty' value for human readability
 */
QVariant DrivePropertiesModel::humanizeSmartAttribute(const SmartAttribute& attr) const
{
  switch(attr.pretty_unit) {
    case 0: return QVariant(i18nc("value type is unknown", "unknown")); break;
    case 1: return QVariant(attr.pretty); break;
    case 2: return QVariant(Humanize::duration(attr.pretty, "ms")); break;
    case 3: return QVariant(i18n("%1 sectors", QString::number(attr.pretty))); break;
    case 4: return QVariant(Humanize::temperature(attr.pretty)); break;
    default: return QVariant();
  }
}



/*
 * Handle the headers of the model
 */
QVariant DrivePropertiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation != Qt::Horizontal)
    return QVariant();

  if(role == Qt::DisplayRole) {
      return QVariant(headerLabels.at(section));

  } else if(role == Qt::FontRole) {
    QFont font;
    font.setBold(true);
    return QVariant(font);
  }

  return QVariant();
}



/*
 * Handle config change
 */
void DrivePropertiesModel::configChanged()
{
  sensitiveAttributes = DiskMonitorSettings::sensitiveAttributes();
}
