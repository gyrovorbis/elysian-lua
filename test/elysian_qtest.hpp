#ifndef ELYSIAN_QTEST_HPP
#define ELYSIAN_QTEST_HPP

#include <QtTest>
#include <QList>
#include <QStack>
#include <QMutex>


/* Expects for ParentTestSet to be a typedef/alias to parent,
 * and expects to be used within "private slots:" declaration. */
#define ELYSIAN_INHERIT_TEST_CASE(NAME) \
    void NAME(void) { return ParentTestSet::NAME(); }

namespace elysian {

class UnitTestSuite;

class UnitTestSet: public QObject {
Q_OBJECT
public:
    void setTestSuite(UnitTestSuite* pSuite);
protected:
    UnitTestSuite* getSuite(void) const;
private:
    UnitTestSuite* m_pSuite = nullptr;

};


class UnitTestSuite: public QObject {
    friend int      main(const QStringList);
protected:
    QList<QObject*> m_testQueue;
    QStack<QString> m_subTestStack;
    QMutex          m_mutex;
    unsigned        m_testSetsRun = 0;
    unsigned        m_testSetsFailed = 0;

public:
    void            enqueueTestSet(QObject* object);
    void            enqueueNextTestSet(QObject* object);

    unsigned        getTestSetsFailed(void) const;
    unsigned        getTestSetsRun(void) const;
    unsigned        getTestSetsPassed(void) const;

    bool            exec(int argc, char* argv[]);
};


inline void UnitTestSet::setTestSuite(UnitTestSuite* pSuite) { m_pSuite = pSuite; }
inline UnitTestSuite* UnitTestSet::getSuite(void) const { return m_pSuite; }

inline unsigned UnitTestSuite::getTestSetsFailed(void) const { return m_testSetsFailed; }
inline unsigned UnitTestSuite::getTestSetsRun(void) const { return m_testSetsRun; }
inline unsigned UnitTestSuite::getTestSetsPassed(void) const { return getTestSetsRun() - getTestSetsFailed(); }

inline void UnitTestSuite::enqueueNextTestSet(QObject *object) {
    m_testQueue.push_front(object);
}

inline void UnitTestSuite::enqueueTestSet(QObject* object) {
   m_testQueue.push_back(object);
}

inline bool UnitTestSuite::exec(int argc, char* argv[]) {

    auto ASSERT_TEST = [&](QObject* obj) {
         m_testSetsFailed += QTest::qExec(obj, argc, argv);
         delete obj;
     };

    while(m_testQueue.size()) {
        ++m_testSetsRun;

        QObject* pCurrent =  m_testQueue.takeFirst();
        UnitTestSet* pSet = qobject_cast<UnitTestSet*>(pCurrent);
        if(pSet) {
            pSet->setTestSuite(this);
        }

        ASSERT_TEST(pCurrent);
    }

    qDebug() << QString("====== TEST SET TOTALS: %1 PASSED, %2 FAILED ======").arg(getTestSetsPassed()).arg(getTestSetsFailed());

    return !m_testSetsFailed;
}



}



#endif // ELYSIAN_QTEST_HPP
