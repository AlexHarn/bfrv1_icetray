
from contextlib import contextmanager
from smtplib import SMTP
from email.MIMEText import MIMEText


class EmailException(Exception):
    pass


@contextmanager
def smtp_context(server):
    s = SMTP()
    try:
        s.connect(server)
        yield s
    finally:
        s.close()


def sendMessage(to_list, sender, subject, server, body):

    if to_list is None:
        return
    msg = MIMEText(body)
    msg['Subject'] = subject
    msg['From'] = sender
    msg['Reply-to'] = sender
    msg['To'] = ', '.join(to_list)
    try:
        with smtp_context(server) as s:
            s.sendmail(sender, to_list, msg.as_string())
    except socketerror, e:
        raise EmailException("Can't send email: socket error '%s'" % e)
