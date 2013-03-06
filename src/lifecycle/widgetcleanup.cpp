#include "widgetcleanup.h"
#include "widgetutilities.h"

#include <QDir>

WidgetCleanup::WidgetCleanup( WidgetInformation &widgetInfo )
    : WidgetOperation( widgetInfo )
{
    
}

WidgetCleanup::~WidgetCleanup()
{
    
}

void WidgetCleanup::execute()
{    
    
    emit completed();
}

void WidgetCleanup::restore()
{
    
}

WidgetErrorType WidgetCleanup::finalize()
{
    LOGIT("WidgetCleanup::finalize()");
	QString widgetBackupPath = m_WidgetInfo[ EPropertyBackupDirectory ].toString();
    
    WidgetUtilities::removeDirectory( widgetBackupPath );
    
    return EErrorNone;
}
