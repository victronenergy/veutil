#pragma once

// Provide a dummy for the non declarative builds
class QDeclarativeParserStatus {
public:
    QDeclarativeParserStatus() {}
    virtual ~QDeclarativeParserStatus() {}

    virtual void classBegin() = 0;
    virtual void componentComplete() = 0;
};

Q_DECLARE_INTERFACE(QDeclarativeParserStatus, "com.victronenergy.CompatParserStatus")
