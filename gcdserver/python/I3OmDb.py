
from MySQLdb import connect, OperationalError
from contextlib import contextmanager
from datetime import datetime
from time import mktime
from threading import Lock


class BadArgumentException(Exception):
    """
    Used in the case where a non-existent field name is specified in a
    query - see the run summary code
    """
    pass


class RepeatedMySQLConnectFailures(Exception):
    """
    Raised if connectRepeated fails to connect to the database on the
    specified host after 4 retries
    """
    pass


MYSQL_TIME_FORMAT = '%Y-%m-%d %H:%M:%S'


def connectRepeated(user, host, passwd=None):
    """
    Tweak to address https://bugs.icecube.wisc.edu/view.php?id=4057.
    """
    RETRIES = 4
    LOST_CONNECTION = 2013
    NO_DATABASE = 1046
    DB_GONE_AWAY = 2006
    for _ in range(RETRIES):
        try:
            if passwd is not None:
                return connect(user=user, host=host, passwd=passwd)
            else:
                return connect(user=user, host=host)
        except OperationalError, e:
            if e[0] in (LOST_CONNECTION,
                        NO_DATABASE,
                        DB_GONE_AWAY):
                pass  # ... try again...
            else:
                raise
    raise RepeatedMySQLConnectFailures()


def cvt_var(x):
    if x is None:
        return "NULL"
    elif type(x) is datetime:
        return '%s' % x.strftime(MYSQL_TIME_FORMAT)
    else:
        return str(x)


class I3OmDb(object):
    """
    Small 'model' to encapsulate online database functionality. Since
    LiveControl has no other Web or database connections, we do not
    use the Django ORM but rather wrap the needed database
    functionality ourselves, below.

    We create a separate connection for each transaction. It's not
    optimal performance-wise, but for this API there isn't that much
    need for performance, and it's trivially thread-safe.
    JB: Stolen from I3Live!
    """
    def __init__(self, host='dbs2', db='I3OmDb', user='www'):
        if host == None:
            raise BadArgumentException("host argument was None")
        if db == None:
            raise BadArgumentException("db argument was None")
        if user == None:
            raise BadArgumentException("user was None")

        self.host = host
        self.db = db
        self.user = user
        self.__lastSqlSecond = int(mktime(datetime.utcnow().timetuple()))
        self.__lastSqlIndex = 0
        self.__lock = Lock()
        self.__run_summary_fields = None

    def connect(self):
        connection = connectRepeated(self.user, self.host)
        cursor = connection.cursor()
        try:
            cursor.execute('USE %s;' % self.db)
        except OperationalError:
            cursor.execute('CREATE DATABASE %s;' % self.db)
            cursor.execute('USE %s;' % self.db)
        return connection, cursor

    @contextmanager
    def db_context(self):
        """
        Send all exceptions back to the caller, but be sure to rollback()
        in case of commit error and be sure to close the connection if
        resources were allocated.
        """
        conn, curs = self.connect()
        try:
            yield curs
            try:
                conn.commit()
            except:
                conn.rollback()
                raise
        finally:
            conn.close()