import os
import re
import sys
import sqlite3
import logging

DBNAME = 'mll.db3'

ALGS_PATH = '../classifiers/'

CREATE_ALG_TABLE = """
  CREATE TABLE IF NOT EXISTS [mll_alg](
      [id] INTEGER
      , [name] TEXT
      , [author] TEXT
      , [description] TEXT      
      , [algsynonim] TEXT
      , [password] TEXT
      , [path] TEXT
      , PRIMARY KEY ([id] AUTOINCREMENT));
"""

SYNCDB_SCRIPTS = [CREATE_ALG_TABLE,]

logger = logging

class Alg(object):
  
  def __init__(self
              , id = 0
              , name = 'UNDEFINED'
              , author = ''
              , description = ''
              , algsynonim = ''
              , password = ''
              , path = ''):
    self.id = id
    self.name = name
    self.author = author
    self.description = description
    self.algsynonim = algsynonim
    self.password = password
    self.path = path

def sync():
  """Synchronizes database"""  
  logger.debug('Synchronizing databse \'{0}\'...'.format(DBNAME))
  
  conn = _connect(DBNAME)  
  if not conn: return
    
  curs = conn.cursor()
  for script in SYNCDB_SCRIPTS:
    _execute(curs, script)      
  
  curs.close()
  conn.commit()  
  conn.close()
  
def load_algs(path=ALGS_PATH):
  """Loads algs form the path and inserts them into db"""
  
  logger.debug('Loading algs from \'{0}\'...'.format(path))
  pattern = re.compile(r'REGISTER_CLASSIFIER.*?\((.*?)\);', re.DOTALL)
     
  for root, dirs, files in os.walk(path):
    if '.svn' in dirs:
      dirs.remove('.svn')  # don't visit SVN directories
    
    for name in files:
      if re.match(r'.*\.c$|.*\.cpp$|.*\.cc$', name):        
        file = open(os.path.join(root, name))
        text = file.read()
        file.close()
        
        match = pattern.search(text)        
        if match:
          fields = match.group(1).replace('\t', ' ')
          fields = fields.replace('\r', ' ')
          fields = fields.replace('\n', ' ')
          fields = fields.split(',')          
          for i in range(len(fields)):
            fields[i] = fields[i].strip().strip('\"')
            
          password = ''
          match = re.search(r'#define[ \t]*?PASSWORD[ \t]*?\"(.*?)\"',text)
          if match:
            password = match.group(1)
          
          algsynonim = ''  
          match = re.search(r'#define[ \t]*?ALGSYNONIM[ \t]*?\"(.*?)\"',text)
          if match:
            algsynonim = match.group(1)          
          
          logger.debug('Alg found: \'{0}\',\'{1}\''.format(fields[1], fields[2]))
          alg = Alg(name=fields[1]
                          , author=fields[2]
                          , description=fields[3]
                          , algsynonim=algsynonim
                          , password=password
                          , path=os.path.join(root, name).replace('\\','/'))
          alg.id = alg_exists(alg.name, alg.author)
          if not alg.id:
            insert_alg(alg)
          else:
            logger.debug('\'{0}\' is already exists'.format(fields[1]))
            update_alg(alg)            
          
def select_algs():
  logger.debug('Selecting algs...')
  
  conn = _connect(DBNAME)  
  if not conn: return
  
  algs = []
  curs = conn.cursor()
  if _execute(curs, """SELECT [id]
                            , [name]
                            , [author]
                            , [description]
                            , [algsynonim]
                            , [password]
                            , [path] FROM [mll_alg]"""):
    for row in curs:
      algs.append(
        Alg(int(row[0]), row[1], row[2], row[3], row[4], row[5], row[6]))
        
  curs.close()
  conn.close()
   
  return algs
  
def insert_alg(alg):
  logger.debug('Inserting alg \'{0}\'...'.format(alg.name))
  
  conn = _connect(DBNAME)  
  if not conn: return
  
  curs = conn.cursor()
  if _execute(curs, """INSERT INTO [mll_alg]([name]
                            , [author]
                            , [description]
                            , [algsynonim]
                            , [password]
                            , [path]) VALUES(?,?,?,?,?,?)"""
                  , [alg.name
                    , alg.author
                    , alg.description
                    , alg.algsynonim
                    , alg.password
                    , alg.path]):
    conn.commit()
        
  curs.close()
  conn.close()
  
def update_alg(alg):
  logger.debug('Updating alg \'{0}\'...'.format(alg.name))
  
  conn = _connect(DBNAME)  
  if not conn: return
  
  curs = conn.cursor()
  if _execute(curs, """UPDATE [mll_alg] SET [name]=?
                            , [author]=?
                            , [description]=?
                            , [algsynonim]=?
                            , [password]=?
                            , [path]=? WHERE [id]=?"""
                  , [alg.name
                    , alg.author
                    , alg.description
                    , alg.algsynonim
                    , alg.password
                    , alg.path
                    , alg.id]):
    conn.commit()
        
  curs.close()
  conn.close()
  
def delete_alg(alg):
  logger.debug('Deleting alg with \'{0}\' id...'.format(alg.id))
  
  conn = _connect(DBNAME)  
  if not conn: return
  
  curs = conn.cursor()
  if _execute(curs, 'DELETE FROM [mll_alg] WHERE [id]=?', [alg.id]):
    conn.commit()
        
  curs.close()
  conn.close()
  
def alg_exists(name, author):
  conn = _connect(DBNAME)  
  if not conn: return
  
  result = None
  curs = conn.cursor()
  if _execute(curs, 'SELECT [id] FROM [mll_alg] WHERE [name]=? AND [author]=?', [name, author]):
    result = curs.fetchone()
    if result:
      result = result[0]
        
  curs.close()
  conn.close()
      
  return result
  
def _connect(dbname):
  conn = None
  try:
    conn = sqlite3.connect(dbname)
  except:
    logger.error(
      '\'{0}\' connection failed: {1}'.format(dbname, sys.exc_info()[0]))
  return conn
    
def _execute(curs, script, args=[]):
  try:
    curs.execute(script, args)
  except:
    logger.error(
      'Can\'t execute script \'{0}\'\n{1}'.format(script, sys.exc_info()[0]))
    return False
  return True
  
def _cmd_usage():
  """Prints supported commands info"""
  commands = {'sync'    :['Creates db if it doesn\'t exist',
                          {}]
              ,'list'   :['Lists all algs from the database',
                          {}]
              ,'load'   :['Loads algs from the given path',
                          {'-p':'path to the algorithms directory (optional)'}]
              ,'delete' :['Delets alg with given Id',
                          {'-id':'alg id'}]
  }
  if len(sys.argv) == 1:
    print('Supported commands:')
    for command, cdescr in commands.items():
      print('{0:7} {1}'.format(command, cdescr[0]))
      for param, pdescr in cdescr[1].items():
        print('\t{0}: {1}'.format(param, pdescr))    
        
def _cmd_list():
  algs = select_algs()
  print('{0:3}|{1:15}|{2:15}|{3:30}|{4:10}|{5}'.format('Id'
            , 'Name'
            , 'Author'
            , 'Algsynonim'
            , 'Password'
            , 'Path'))
  for alg in algs:
    print('{0:3}|{1:15}|{2:15}|{3:30}|{4:10}|{5}'.format(alg.id
            , alg.name
            , alg.author
            , alg.algsynonim
            , alg.password
            , alg.path))  
            
def _cmd_load(path):
  load_algs(path)
  
def _cmd_delete(id):
  delete_alg(Alg(id=id))
  
if __name__ == '__main__':
  
  logger = logging.getLogger('mlldb')
  logger.setLevel(logging.DEBUG)
  logger.addHandler(logging.StreamHandler())
  
  if len(sys.argv) == 1:
    _cmd_usage()
  elif(sys.argv[1] == 'sync'):
    sync()
  elif(sys.argv[1] == 'list'):
    _cmd_list()
  elif(sys.argv[1] == 'load'):
    path = ALGS_PATH
    for i in range(2,len(sys.argv),2):
      if sys.argv[i] == '-p':
        path = sys.argv[i + 1]
    if path:
      _cmd_load(path)
    else:
      _cmd_usage()
  elif(sys.argv[1] == 'delete'):
    id = 0
    for i in range(2,len(sys.argv),2):
      if sys.argv[i] == '-id':
        id = int(sys.argv[i + 1])
    if id:
      _cmd_delete(id)
    else:
      _cmd_usage()
  else:
    _cmd_usage()
    
  
