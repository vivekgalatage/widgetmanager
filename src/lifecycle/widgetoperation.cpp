#include "widgetoperation.h"

WidgetOperation::WidgetOperation( WidgetInformation& widgetInfo )
    : m_WidgetInfo( widgetInfo ),
      m_Mode( EModeInvalid ),
      m_Finalized( false )
{
}

WidgetOperation::~WidgetOperation()
{    
}

WidgetErrorType WidgetOperation::complete()
{
    m_Finalized = true;
    return finalize();
}

WidgetErrorType WidgetOperation::finalize()
{
    return EErrorNone;
}

void WidgetOperation::setMode( const OperationMode& mode )
{
    m_Mode = mode;
}

void WidgetOperation::interactiveProperties( QMap<WidgetPropertyType, QVariant> &/*properties*/ )
{

}

void WidgetOperation::cancel()
{
    restore();
}

const QList<WidgetOperationData>& WidgetOperation::subOperations()
{
    return m_SubOperations;
}

bool WidgetOperation::addSuboperation( const WidgetOperationType& type, WidgetOperation *operation )
{
    WidgetOperationData data( type, operation );
    m_SubOperations.append( data );
    return true;
}

bool WidgetOperation::addSuboperation( WidgetOperation *operation )
{
    WidgetOperationData data( EOperationAny, operation );
    m_SubOperations.append( data );
    return true;
}

bool WidgetOperation::clearSubOperations()
{
    m_SubOperations.clear();
    return true;
}

bool WidgetOperation::isFinalized()
{
    return m_Finalized;
}

