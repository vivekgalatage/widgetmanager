#ifndef WIDGETOPERATION_H
#define WIDGETOPERATION_H

#include "widgetinformation.h"

#include <QObject>
#include <QList>
#include <QVariant>

class WidgetOperation;
class WidgetOperationData
{
public:
    WidgetOperationData( WidgetOperationType type, WidgetOperation *operation )
        : m_type( type ),
          m_operation( operation )
    {
    }

    bool operator ==( const WidgetOperationData& rhs )
    {
        return ( this->m_operation == rhs.m_operation && this->m_type == rhs.m_type );
    }

public:
    WidgetOperationType m_type;
    WidgetOperation* m_operation;
};

//! Abstract class to represent various operations on a Widget.
/*!
  The class represents the various operations on a Widget. During the widget installation or
  uninstallation a widget goes through various operations performed on it such as in extraction of widget
  content, validation the widget configuration document etc. So the class WidgetOperation acts as a interface
  to provide all such operations.
 */
class WidgetOperation : public QObject
{
    Q_OBJECT
public:
    enum OperationMode
    {
        EModeInvalid = -1,
        EModeInteractive,
        EModeNonInteractive
    };

public:
    //! Constructor
    /*!
      Constructor for the WidgetOperation which stores the WidgetInformation as reference.
      @param widgetInfo reference to WidgetInformation
      */
    explicit WidgetOperation( WidgetInformation &widgetInfo );

    //! Destructor
    /*!
      Virtual destructor for all WidgetOperation
      */
    virtual ~WidgetOperation();

    //! Abstract method for executing the operation
    /*!
      The method is invoked in order to execute the WidgetOperation
      */
    virtual void execute() = 0;
    
    
    virtual WidgetErrorType finalize();
    
    WidgetErrorType complete();
    
    //! Abstract method for restoring the operation state
    /*!
      The method is invoked in case of the operation failure. The implementation should restore the system
      to its previous state
      */
    virtual void restore() = 0;

    //! Abstract method for handling the cancel request by user
    /*!
      The method is invoked in case the user cancels the ongoing operation. The implementation should cancel any pending
      requests and restore the system state if required.
      */
    virtual void cancel();
    

    virtual void interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties );
    
    void setMode( const OperationMode& mode );


    const QList<WidgetOperationData>& subOperations();

    bool clearSubOperations();

    bool addSuboperation( const WidgetOperationType& type, WidgetOperation *operation = NULL );
    
    bool addSuboperation( WidgetOperation *operation );
    
    bool isFinalized();

signals:

    void interactiveRequest( QMap<WidgetPropertyType, QVariant> &properties );
    
    void completed();
    
    void aborted( WidgetErrorType );

private:
    /*!
      Avoid the copying of the class by making default constructor, copy constructor and assignment operator private
      */
    WidgetOperation();
    explicit WidgetOperation( const WidgetOperation& rhs );
    WidgetOperation & operator = ( const WidgetOperation& rhs );
        
protected:
    //! Reference to WidgetInformation
    WidgetInformation &m_WidgetInfo;

    OperationMode m_Mode;
    
    bool m_Finalized;

    QList<WidgetOperationData> m_SubOperations;
};

#endif // WIDGETOPERATION_H
