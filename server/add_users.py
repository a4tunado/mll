import os
import sys
import random
import string
import subprocess
import logging

logger = logging

def _gen_password():
  password = ''
  for i in range(7):
    r = random.randint(0,3)
    if r > 2:
      password += string.ascii_uppercase[random.randint(0,25)]
    elif r > 1:
      password += string.ascii_lowercase[random.randint(0,25)]
    else:
      password += string.digits[random.randint(0,9)]
  return password
  
def _add_user(name, password):
  logger.info('Inserting user {0}...'.format(name))

  args = ['htpasswd'
          ,'-b'
          ,'/etc/apache2/dav_svn.passwd'
          ,name
          ,password]
          
  popen = subprocess.Popen(args)
  popen.wait()
  
  if popen.returncode: 
    logger.error('Can\'t create svn user {0}'.format(name))
    
  args = ['htpasswd'
         ,'-b'
	 ,'/etc/apache2/trac.passwd'
	 ,name
	 ,password]
	 
  popen = subprocess.Popen(args)
  popen.wait()
  
  if popen.returncode:
    logger.error('Can\'t create trac user {0}'.format(name))
  
def _set_permissions(users, path, permission='rw'):

  file = open('/etc/apache2/dav_svn.authz')
  text = file.read()
  file.close()
  
  file = open('/etc/apache2/dav_svn.authz.bak','w')
  file.write(text)
  file.close()
  
  text.strip('\n')  
  for user in users:
    text += '[{0}]\n{1}=r\n'.format('/mll/branches/public',user[0])
    text += '[{0}/{1}]\n{1}={2}\n'.format(path, user[0], permission)
  
  file = open('/etc/apache2/dav_svn.authz','w')
  file.write(text)
  file.close()
  
def _create_dirs(users, path):

  logger.info('Checking out...')

  args = ['svn'
          ,'co'
          ,'https://shad1.yandex.ru/svn/repo{0}'.format(path)
          , 'temp']
          
  popen = subprocess.Popen(args)
  popen.wait()
  if popen.returncode: 
    logger.error('Can\'t checkout {0}'.format(path))

  logger.info('Creating folders...')
  for user in users:
    if not os.path.exists(os.path.join('temp', user[0])):
      os.mkdir(os.path.join('temp',user[0]))
      args = ['svn'
              ,'add'
              ,os.path.join('temp',user[0])]          
      popen = subprocess.Popen(args)
      popen.wait()
      if popen.returncode: 
        logger.error('Can\'t add {0}'.format(path))
    
  logger.info('Checking in...')

  args = ['svn'
          ,'ci'
          ,'temp'
          , '-m'
          , 'User folders added']
          
  popen = subprocess.Popen(args)
  popen.wait()
  if popen.returncode: 
    logger.error('Checkin error')
    
  logger.info('Removing temp dir...')
  
  for root, dirs, files in os.walk('temp', topdown=False):
    for name in files:
      os.remove(os.path.join(root,name))
    for name in dirs:
      os.rmdir(os.path.join(root,name))
  
  logger.info('Dirs created successfully.')
  
def _backup_passwd():
  file = open('/etc/apache2/dav_svn.passwd')
  text = file.read()
  file.close()
  
  file = open('/etc/apache2/dav_svn.passwd.bak', 'w')
  file.write(text)
  file.close()
  
def _get_logger():
  logger = logging.getLogger('Default')  
  logger.setLevel(logging.DEBUG)  
  stream_handler = logging.StreamHandler()  
  formatter = logging.Formatter("%(asctime)s\t%(levelname)s\t%(message)s")
  stream_handler.setFormatter(formatter)
  logger.addHandler(stream_handler) 
  return logger

if __name__ == "__main__":

  path = '/mll/branches/poligon/classifiers'
  input = 'add_users.in'
  output = 'add_users.out'  
  
  if (len(sys.argv) > 1):
    input = sys.argv[1]
  
  if (len(sys.argv) > 2):
    path = sys.argv[2]
  
  if (len(sys.argv) > 3):
    output = sys.argv[3]
    
  logger = _get_logger()
    
  logger.info('input:  {0}'.format(input))
  logger.info('output: {0}'.format(output))
  logger.info('path:   {0}'.format(path))
  
  _backup_passwd()
  
  file = open(input)
  text = file.read()
  file.close()

  users = []  
  lines = text.split('\n')
  for line in lines:
    if line:
      items = line.split(' ')
      name = items[0]
      password = ''
      if len(items) == 2:        
        password = items[1]
      else:
        password = _gen_password()
    
      _add_user(name, password)
      users.append([name, password])
      
  _set_permissions(users, path)
  _create_dirs(users, path)
  
  logger.info('Writing output...')
  
  text = ''
  for user in users:
    text += '{0} {1}\n'.format(user[0], user[1])
  
  file = open(output,'w')
  file.write(text)
  file.close()
  
            