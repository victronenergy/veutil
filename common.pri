# Mind it, this file might be included multiple times
INCLUDEPATH *= "$$PWD/inc"

*g++* {
    equals(QT_MAJOR_VERSION, 4) {
        # suppress the mangling of va_arg has changed for gcc 4.4
        QMAKE_CXXFLAGS *= -Wno-psabi -std=c++11

        # gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
        QMAKE_CXXFLAGS *= -Wno-unused-local-typedefs

        QMAKE_CXX *= -Wno-class-memaccess
        QMAKE_CXX *= -Wno-deprecated-copy
    }

    equals(QT_MAJOR_VERSION, 5) {
        QMAKE_CXX *= -Wno-deprecated-copy
    }
}
