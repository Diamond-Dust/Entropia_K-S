# -*- mode: python ; coding: utf-8 -*-

block_cipher = None


a = Analysis(['Entropy.py'],
             pathex=['E:\\Moje programy\\Entropia - Grzegorz Hajdukiewicz\\Program'],
             binaries=[],
             datas=[('E:\\Moje programy\\Entropia - Grzegorz Hajdukiewicz\\Program\\default.ini', '.'),
             ('E:\\Moje programy\\Entropia - Grzegorz Hajdukiewicz\\Program\\entropy.ppm', '.'),
             ('E:\\Moje programy\\Entropia - Grzegorz Hajdukiewicz\\Program\\error.ppm', '.'),
             ('E:\\Moje programy\\Entropia - Grzegorz Hajdukiewicz\\Program\\warning.ppm', '.')],
             hiddenimports=[],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher,
             noarchive=False)
pyz = PYZ(a.pure, a.zipped_data,
             cipher=block_cipher)
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          [],
          name='EntropyLMDD',
          debug=False,
          bootloader_ignore_signals=False,
          strip=False,
          upx=True,
          upx_exclude=[],
          runtime_tmpdir=None,
          console=False, icon='E:\\Moje programy\\Entropia - Grzegorz Hajdukiewicz\\Program\\entropy.ico' )

import shutil
shutil.copyfile('settings.ini', '{0}/settings.ini'.format(DISTPATH))
