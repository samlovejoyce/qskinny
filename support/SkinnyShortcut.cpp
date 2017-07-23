#include "SkinnyShortcut.h"

#include <QskSkinFactory.h>
#include <QskShortcut.h>
#include <QskSetup.h>
#include <QskWindow.h>
#include <QskAspect.h>
#include <QskSkin.h>
#include <QskControl.h>
#include <QskSkinTransition.h>

#include <QQuickItem>
#include <QKeySequence>
#include <QGuiApplication>
#include <QSGNode>
#include <QDebug>

#include <unordered_map>

namespace
{
    class SkinTransition : public QskSkinTransition
    {
    protected:
        virtual void updateSkin( QskSkin*, QskSkin* ) override final
        {
            // nop
        }
    };
}

SkinnyShortcut::SkinnyShortcut( QObject* parent ):
    QObject( parent )
{
}

void SkinnyShortcut::enable( Types types )
{
    static SkinnyShortcut s_shortcut;

    if ( types & Quit )
    {
        // QKeySequence::Quit crashes the application
        // when not being implemented by the platform !!

        QskShortcut::addShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ),
            QGuiApplication::instance(), SLOT( quit() ) );
    }

    if ( types & RotateSkin )
    {
        QskShortcut::addShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ),
            &s_shortcut, SLOT( rotateSkin() ) );
    }

    if ( types & DebugBackground )
    {
        QskShortcut::addShortcut( QKeySequence( Qt::CTRL + Qt::Key_B ),
            &s_shortcut, SLOT( showBackground() ) );
    }

    if ( types & DebugStatistics )
    {
        QskShortcut::addShortcut( QKeySequence( Qt::CTRL + Qt::Key_K ),
            &s_shortcut, SLOT( debugStatistics() ) );
    }
}

void SkinnyShortcut::rotateSkin()
{
    const QStringList names = Qsk::skinNames();
    if ( names.size() <= 1 )
        return;

    int index = names.indexOf( qskSetup->skinName() );
    index = ( index + 1 ) % names.size();

    QskSkin* oldSkin = qskSetup->skin();
    if ( oldSkin->parent() == qskSetup )
        oldSkin->setParent( nullptr ); // otherwise setSkin deletes it

    QskSkin* newSkin = qskSetup->setSkin( names[ index ] );

    SkinTransition transition;

    //transition.setMask( QskAspect::Color ); // Metrics are flickering -> TODO
    transition.setSourceSkin( oldSkin );
    transition.setTargetSkin( newSkin );
    transition.setAnimation( 500 );

    transition.process();

    if ( oldSkin->parent() == nullptr )
        delete oldSkin;
}

void SkinnyShortcut::showBackground()
{
#if 0
    const QVector< QByteArray > sgDebugModes =
        { "clip", "overdraw", "changes", "batches" };
#else
    const QVector< QByteArray > sgDebugModes = { "overdraw" };
#endif

    static int id = 0;

    id = ( id + 1 ) % ( sgDebugModes.count() + 2 );

    bool forceBackground = false;
    QByteArray scengraphDebugMode;

    if ( id == 0 )
    {
        // nothing
    }
    else if ( id == 1 )
    {
        forceBackground = true;
    }
    else
    {
        scengraphDebugMode = sgDebugModes[ id - 2 ];
    }

    qskSetup->setControlFlag( QskSetup::DebugForceBackground, forceBackground );

    for ( auto window : QGuiApplication::topLevelWindows() )
    {
        if ( QskWindow* w = qobject_cast< QskWindow* >( window ) )
        {
            if ( w->customRenderMode() != scengraphDebugMode )
            {
                w->setCustomRenderMode( scengraphDebugMode );
                w->update();
            }
        }
    }
}

static inline void countNodes( const QSGNode* node, int& counter )
{
    if ( node )
    {
        counter++;

        for ( auto child = node->firstChild();
            child != nullptr; child = child->nextSibling() )
        {
            countNodes( child, counter );
        }
    }
}

static void countNodes( const QQuickItem* item, int& counter )
{
    const auto itemNode = QskControl::itemNode( item );

    if ( itemNode )
    {
        auto node = QskControl::paintNode( item );
        if ( node )
        {
            countNodes( node, counter );
            while ( node != itemNode )
            {
                counter++;
                node = node->parent();
            }
        }
        else
        {
            counter++;
            // clipNode/opacityNode ???
        }
    }
}

static void countItems( const QQuickItem* item, int counter[4] )
{
    if ( item )
    {
        counter[0]++;

        int nodeCounter = 0;
        countNodes( item, nodeCounter );

        counter[2] += nodeCounter;

        if ( item->isVisible() )
        {
            counter[1]++;
            counter[3] += nodeCounter;
        }

        for ( const auto* child : item->childItems() )
            countItems( child, counter );
    }
}

void SkinnyShortcut::debugStatistics()
{
    for ( auto window : QGuiApplication::topLevelWindows() )
    {
        const auto w = qobject_cast< const QQuickWindow* >( window );
        if ( w == nullptr )
            continue;

        int counter[4] = {};
        countItems( w->contentItem(), counter );

        qDebug() << w << "\n\titems:" << counter[0] << "visible" << counter[1]
            << "\n\tnodes:" << counter[2] << "visible" << counter[3];
    }
}

#include "moc_SkinnyShortcut.cpp"