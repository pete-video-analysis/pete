#!python

VariantDir('build', 'src', duplicate=False)

opts = Variables([], ARGUMENTS)

env = Environment()

opts.Add(PathVariable('target_path', 'The path where the lib is installed.', 'build/'))
opts.Add(PathVariable('target_name', 'The library name.', 'libpete', PathVariable.PathAccept))
opts.Add(BoolVariable('use_PIC', 'Use the position independent code compilation option', False))

opts.Update(env)

libs = ['m']
sources = Glob('build/*.c')

env.Append(CPPPATH='include')
env.Append(LIBS=libs)

if(env['use_PIC']):
	env.Append(CPPFLAGS='-fPIC')

env.StaticLibrary(target=env['target_path'] + env['target_name'] , source=sources)

Help(opts.GenerateHelpText(env))