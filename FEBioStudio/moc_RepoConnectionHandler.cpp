/****************************************************************************
** Meta object code from reading C++ file 'RepoConnectionHandler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "RepoConnectionHandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RepoConnectionHandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CRepoConnectionHandler_t {
    QByteArrayData data[9];
    char stringdata0[100];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CRepoConnectionHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CRepoConnectionHandler_t qt_meta_stringdata_CRepoConnectionHandler = {
    {
QT_MOC_LITERAL(0, 0, 22), // "CRepoConnectionHandler"
QT_MOC_LITERAL(1, 23, 12), // "connFinished"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 14), // "QNetworkReply*"
QT_MOC_LITERAL(4, 52, 1), // "r"
QT_MOC_LITERAL(5, 54, 15), // "sslErrorHandler"
QT_MOC_LITERAL(6, 70, 5), // "reply"
QT_MOC_LITERAL(7, 76, 16), // "QList<QSslError>"
QT_MOC_LITERAL(8, 93, 6) // "errors"

    },
    "CRepoConnectionHandler\0connFinished\0"
    "\0QNetworkReply*\0r\0sslErrorHandler\0"
    "reply\0QList<QSslError>\0errors"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CRepoConnectionHandler[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x08 /* Private */,
       5,    2,   27,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 7,    6,    8,

       0        // eod
};

void CRepoConnectionHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CRepoConnectionHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 1: _t->sslErrorHandler((*reinterpret_cast< QNetworkReply*(*)>(_a[1])),(*reinterpret_cast< const QList<QSslError>(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CRepoConnectionHandler::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_CRepoConnectionHandler.data,
    qt_meta_data_CRepoConnectionHandler,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CRepoConnectionHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CRepoConnectionHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CRepoConnectionHandler.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CRepoConnectionHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE