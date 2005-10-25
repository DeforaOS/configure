/* configure.c */



#include <System.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

ARRAY(Config *, config);


#define PROJECT_CONF "project.conf"
#define MAKEFILE "Makefile"


/* types */
typedef enum _TargetType { TT_BINARY = 0, TT_LIBRARY, TT_OBJECT, TT_UNKNOWN
} TargetType;
#define TT_LAST TT_UNKNOWN
String * sTargetType[TT_LAST] = { "binary", "library", "object" };
static TargetType _target_type(String const * target)
{
	int i;

	for(i = 0; i < TT_LAST; i++)
		if(string_compare(sTargetType[i], target) == 0)
			return i;
	return TT_UNKNOWN;
}

typedef enum _ObjectType { OT_C_SOURCE = 0, OT_ASM_SOURCE, OT_UNKNOWN
} ObjectType;
#define OT_LAST OT_UNKNOWN
String * sObjectType[OT_LAST] = { "c", "S" };
static String * _source_extension(String * source)
{
	int len;

	for(len = string_length(source)-1; len >= 0; len--)
		if(source[len] == '.')
			return &source[len+1];
	return NULL;
}
static ObjectType _object_type(String * extension)
{
	int i;

	for(i = 0; i < OT_LAST; i++)
		if(string_compare(sObjectType[i], extension) == 0)
			return i;
	return OT_UNKNOWN;
}


/* configure */
/* types */
typedef int Prefs;
#define PREFS_n		0x1
#define PREFS_v		0x2


/* functions */
static int _configure_error(char const * message, int ret);
static int _configure_load(Prefs * prefs, char const * directory,
		configArray * ca);
static int _configure_do(Prefs * prefs, configArray * ca);
static int _configure(Prefs * prefs, char const * directory)
{
	configArray * ca;
	int ret;
	int i;
	Config * p;
	int prefs2;

	if((ca = configarray_new()) == NULL)
		return _configure_error("libSystem", 2);
	ret = _configure_load(prefs, directory, ca);
	if(ret == 0)
	{
		if(*prefs & PREFS_n)
			ret = _configure_do(prefs, ca);
		else
		{
			prefs2 = PREFS_n;
			if(_configure_do(&prefs2, ca) == 0)
				ret = _configure_do(prefs, ca);
		}
	}
	for(i = array_count(ca); i > 0; i--)
	{
		array_get_copy(ca, i - 1, &p);
		config_delete(p);
	}
	array_delete(ca);
	return ret;
}

static int _configure_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "configure: ");
	perror(message);
	return ret;
}


static int _load_subdirs(Prefs * prefs, char const * directory,
		configArray * ca, String * subdirs);
static int _configure_load(Prefs * prefs, String const * directory,
		configArray * ca)
{
	int ret = 0;
	Config * config;
	String * path;
	String * subdirs = NULL;

	if((path = string_new(directory)) == NULL)
		return _configure_error(directory, 1);
	if(string_append(&path, "/") != 0
			|| string_append(&path, PROJECT_CONF) != 0
			|| (config = config_new()) == NULL)
	{
		string_delete(path);
		return _configure_error(directory, 1);
	}
	config_set(config, "", "directory", directory);
	if(*prefs & PREFS_v)
		printf("%s%s%s", "Loading project file ", path, "\n");
	if(config_load(config, path) != 0)
		ret = _configure_error(path, 1);
	else
	{
		array_append(ca, &config);
		subdirs = config_get(config, "", "subdirs");
	}
	string_delete(path);
	if(subdirs == NULL)
		return ret;
	return _load_subdirs(prefs, directory, ca, subdirs);
}

static int _load_subdirs_subdir(Prefs * prefs, char const * directory,
		configArray * ca, char const * subdir);
static int _load_subdirs(Prefs * prefs, char const * directory,
		configArray * ca, String * subdirs)
{
	int ret = 0;
	int i;
	char c;
	String * subdir;
	
	subdir = subdirs;
	for(i = 0; ret == 0; i++)
	{
		if(subdir[i] != ',' && subdir[i] != '\0')
			continue;
		c = subdir[i];
		subdir[i] = '\0';
		ret = _load_subdirs_subdir(prefs, directory, ca, subdir);
		if(c == '\0')
			break;
		subdir[i] = c;
		subdir+=i+1;
		i = 0;
	}
	return ret;
}


static int _load_subdirs_subdir(Prefs * prefs, char const * directory,
		configArray * ca, char const * subdir)
/* FIXME error checking */
{
	int ret;
	String * p;

	p = string_new(directory);
	string_append(&p, "/");
	string_append(&p, subdir);
	ret = _configure_load(prefs, p, ca);
	string_delete(p);
	return ret;
}


static int _do_makefile(Prefs * prefs, Config * config, String * directory,
		configArray * ca, int from, int to);
static int _configure_do(Prefs * prefs, configArray * ca)
{
	int i;
	int cnt = array_count(ca);
	Config * ci;
	String * di;
	int j;
	Config * cj;
	String * dj;

	for(i = 0; i < cnt; i++)
	{
		array_get_copy(ca, i, &ci);
		di = config_get(ci, "", "directory");
		for(j = i; j < cnt; j++)
		{
			array_get_copy(ca, j, &cj);
			dj = config_get(cj, "", "directory");
			if(string_find(dj, di) == NULL)
				break;
		}
		if(_do_makefile(prefs, ci, di, ca, i, j) != 0)
			break;
	}
	return i == cnt ? 0 : 1;
}

static int _makefile_write(Prefs * prefs, Config * config, FILE * fp,
		configArray * ca, int from, int to);
static int _do_makefile(Prefs * prefs, Config * config, String * directory,
		configArray * ca, int from, int to)
{
	String * makefile;
	FILE * fp = NULL;
	int ret = 0;

	makefile = string_new(directory);
	string_append(&makefile, "/");
	string_append(&makefile, MAKEFILE);
	if(!(*prefs & PREFS_n) && (fp = fopen(makefile, "w")) == NULL)
		ret = _configure_error(makefile, 1);
	else
	{
		if(*prefs & PREFS_v)
			printf("%s%s%s%s%s", "Creating ", MAKEFILE, " in ",
					directory, "\n");
		ret += _makefile_write(prefs, config, fp, ca, from, to);
		if(fp != NULL)
			fclose(fp);
	}
	string_delete(makefile);
	return ret;
}

static int _write_variables(Prefs * prefs, Config * config, FILE * fp);
static int _write_targets(Prefs * prefs, Config * config, FILE * fp);
static int _write_objects(Prefs * prefs, Config * config, FILE * fp);
static int _write_clean(Prefs * prefs, Config * config, FILE * fp);
static int _write_distclean(Prefs * prefs, Config * config, FILE * fp);
static int _write_dist(Prefs * prefs, Config * config, FILE * fp,
		configArray * ca, int from, int to);
static int _write_install(Prefs * prefs, Config * config, FILE * fp);
static int _makefile_write(Prefs * prefs, Config * config, FILE * fp,
		configArray * ca, int from, int to)
{
	if(_write_variables(prefs, config, fp) != 0
			|| _write_targets(prefs, config, fp) != 0
			|| _write_objects(prefs, config, fp) != 0
			|| _write_clean(prefs, config, fp) != 0
			|| _write_distclean(prefs, config, fp) != 0
			|| _write_dist(prefs, config, fp, ca, from, to) != 0
			|| _write_install(prefs, config, fp) != 0)
		return 1;
	return 0;
}

static int _variables_package(Prefs * prefs, Config * config, FILE * fp,
		String const * directory);
static int _variables_print(Prefs * prefs, Config * config, FILE * fp,
		char const * input, char const * output);
static int _variables_targets(Prefs * prefs, Config * config, FILE * fp);
static int _variables_executables(Prefs * prefs, Config * config, FILE * fp);
static int _write_variables(Prefs * prefs, Config * config, FILE * fp)
{
	String const * directory = config_get(config, "", "directory");
	int ret = 0;

	ret += _variables_package(prefs, config, fp, directory);
	ret += _variables_print(prefs, config, fp, "subdirs", "SUBDIRS");
	ret += _variables_targets(prefs, config, fp);
	ret += _variables_executables(prefs, config, fp);
	if(!(*prefs & PREFS_n))
		fputc('\n', fp);
	return ret;
}

static int _variables_package(Prefs * prefs, Config * config, FILE * fp,
		String const * directory)
{
	String * package;
	String * version;

	if((package = config_get(config, "", "package")) == NULL)
		return 0;
	if(*prefs & PREFS_v)
		printf("%s%s", "Package: ", package);
	if((version = config_get(config, "", "version")) == NULL)
	{
		if(*prefs & PREFS_v)
			fputc('\n', stdout);
		fprintf(stderr, "%s%s%s", "configure: ", directory,
				": \"package\" needs \"version\"\n");
		return 1;
	}
	if(*prefs & PREFS_v)
		printf("%s%s%s", " ", version, "\n");
	if(fp != NULL)
		fprintf(fp, "%s%s%s%s%s", "PACKAGE\t= ", package,
				"\nVERSION\t= ", version, "\n");
	return 0;
}

static int _variables_print(Prefs * prefs, Config * config, FILE * fp,
		char const * input, char const * output)
{
	String * prints;
	int i;
	char c;

	if(*prefs & PREFS_n)
		return 0;
	if((prints = config_get(config, "", input)) == NULL)
		return 0;
	fprintf(fp, "%s%s", output, "\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		fprintf(fp, " %s", prints);
		if(c == '\0')
			break;
		prints[i] = c;
		prints+=i+1;
		i = 0;
	}
	fputc('\n', fp);
	return 0;
}

static int _variables_targets(Prefs * prefs, Config * config, FILE * fp)
{
	String * prints;
	int i;
	char c;
	String * type;

	if(*prefs & PREFS_n)
		return 0;
	if((prints = config_get(config, "", "targets")) == NULL)
		return 0;
	fprintf(fp, "%s%s", "TARGETS", "\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if((type = config_get(config, prints, "type")) == NULL)
			fprintf(fp, " %s", prints);
		else
			switch(_target_type(type))
			{
				case TT_BINARY:
				case TT_OBJECT:
				case TT_UNKNOWN:
					fprintf(fp, " %s", prints);
					break;
				case TT_LIBRARY:
					fprintf(fp, " %s%s%s%s", prints, ".a ",
							prints, ".so");
					break;
			}
		if(c == '\0')
			break;
		prints[i] = c;
		prints+=i+1;
		i = 0;
	}
	fputc('\n', fp);
	return 0;
}

static int _executables_variables(Config * config, FILE * fp, String * target);
static int _variables_executables(Prefs * prefs, Config * config, FILE * fp)
{
	String * targets;
	int i;
	char c;

	if(*prefs & PREFS_n)
		return 0;
	if((targets = config_get(config, "", "targets")) != NULL)
	{
		for(i = 0;; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			_executables_variables(config, fp, targets);
			if(c == '\0')
				break;
			targets[i] = c;
			targets+=i+1;
			i = 0;
		}
		fprintf(fp, "%s", "RM\t= rm -f\n");
	}
	if(config_get(config, "", "package"))
		fprintf(fp, "%s", "TAR\t= tar cfzv\n");
	if(targets != NULL)
	{
		fprintf(fp, "%s", "MKDIR\t= mkdir -p\n");
		fprintf(fp, "%s", "INSTALL\t= install\n");
	}
	return 0;
}

static int _executables_variables(Config * config, FILE * fp, String * target)
{
	static Config * flag = NULL;
	String * type;
	char done[TT_LAST]; /* FIXME even better if'd be variable by variable */
	String * p;
	TargetType tt;

	if(flag != config)
	{
		flag = config;
		memset(done, 0, sizeof(done));
	}
	if((type = config_get(config, target, "type")) == NULL)
		return 0;
	if(done[(tt = _target_type(type))])
		return 0;
	switch(tt)
	{
		case TT_BINARY:
			if(!done[TT_LIBRARY])
			{
				fprintf(fp, "%s", "PREFIX\t= /usr/local\n");
				fprintf(fp, "%s", "DESTDIR\t=\n");
			}
			fprintf(fp, "%s", "BINDIR\t= $(PREFIX)/bin\n");
			if(!done[TT_LIBRARY])
			{
				fprintf(fp, "%s", "CC\t= cc\n");
				if((p = config_get(config, "", "cflags_force"))
						!= NULL)
					fprintf(fp, "%s%s%s", "CFLAGSF\t= ", p,
							"\n");
				if((p = config_get(config, "", "cflags"))
						!= NULL)
					fprintf(fp, "%s%s%s", "CFLAGS\t= ", p,
							"\n");
			}
			if((p = config_get(config, "", "ldflags_force"))
					!= NULL)
				fprintf(fp, "%s%s%s", "LDFLAGSF= ", p, "\n");
			if((p = config_get(config, "", "ldflags")) != NULL)
				fprintf(fp, "%s%s%s", "LDFLAGS\t= ", p, "\n");
			break;
		case TT_LIBRARY:
			if(!done[TT_LIBRARY])
			{
				fprintf(fp, "%s", "PREFIX\t= /usr/local\n");
				fprintf(fp, "%s", "DESTDIR\t=\n");
			}
			fprintf(fp, "%s", "LIBDIR\t= $(PREFIX)/lib\n");
			if(!done[TT_BINARY])
			{
				fprintf(fp, "%s", "CC\t= cc\n");
				if((p = config_get(config, "", "cflags_force"))
						!= NULL)
					fprintf(fp, "%s%s%s", "CFLAGSF\t= ", p,
							"\n");
				if((p = config_get(config, "", "cflags"))
						!= NULL)
					fprintf(fp, "%s%s%s", "CFLAGS\t= ", p,
							"\n");
			}
			fprintf(fp, "%s", "AR\t= ar rc\n");
			fprintf(fp, "%s", "RANLIB\t= ranlib\n");
			fprintf(fp, "%s", "LD\t= ld -shared\n");
			break;
		case TT_OBJECT:
			/* FIXME */
			break;
		case TT_UNKNOWN:
			break;
	}
	done[tt] = 1;
	return 0;
}

static int _targets_all(Prefs * prefs, Config * config, FILE * fp);
static int _targets_subdirs(Prefs * prefs, Config * config, FILE * fp);
static int _targets_target(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _write_targets(Prefs * prefs, Config * config, FILE * fp)
{
	char * targets = config_get(config, "", "targets");
	char c;
	int i;
	int ret = 0;

	if(_targets_all(prefs, config, fp) != 0
			|| _targets_subdirs(prefs, config, fp) != 0)
		return 1;
	if(targets == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		ret += _targets_target(prefs, config, fp, targets);
		if(c == '\0')
			break;
		targets[i] = c;
		targets+=i+1;
		i = 0;
	}
	return ret;
}

static int _targets_all(Prefs * prefs, Config * config, FILE * fp)
{
	if(*prefs & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\nall:");
	if(config_get(config, "", "subdirs") != NULL)
		fprintf(fp, "%s", " subdirs");
	if(config_get(config, "", "targets") != NULL)
		fprintf(fp, "%s", " $(TARGETS)");
	fprintf(fp, "%s", "\n");
	return 0;
}

static int _targets_subdirs(Prefs * prefs, Config * config, FILE * fp)
{
	String * subdirs;

	if(*prefs & PREFS_n)
		return 0;
	if((subdirs = config_get(config, "", "subdirs")) != NULL)
		fprintf(fp, "%s", "\nsubdirs:\n\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE)) || exit; done\n");
	return 0;
}

static int _target_objs(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _target_library(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _targets_target(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	String * type;
	TargetType tt;
	String * p;

	if((type = config_get(config, target, "type")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", target,
				": no type defined for target\n");
		return 1;
	}
	tt = _target_type(type);
	switch(tt)
	{
		case TT_BINARY:
			if(_target_objs(prefs, config, fp, target) != 0)
				return 1;
			if(*prefs & PREFS_n)
				return 0;
			fprintf(fp, "%s%s", target, "_CFLAGS = $(CFLAGSF)"
					" $(CFLAGS)");
			if((p = config_get(config, target, "cflags")) != NULL)
				fprintf(fp, "%s%s", " ", p);
			fputc('\n', fp);
			fprintf(fp, "%s%s%s%s", target, ": $(", target,
					"_OBJS)\n");
			fprintf(fp, "%s%s%s%s%s", "\t$(CC) $(LDFLAGSF)"
					" $(LDFLAGS) -o ", target, " $(",
					target, "_OBJS)\n");
			break;
		case TT_LIBRARY:
			return _target_library(prefs, config, fp, target);
		case TT_OBJECT:
			if((p = config_get(config, target, "sources")) == NULL)
			{
				fprintf(stderr, "%s%s%s", "configure: ", target,
						" no sources for target\n");
				return 1;
			}
			if(*prefs & PREFS_n)
				return 0;
			fprintf(fp, "%s%s%s%s", target, ": ", p, "\n");
			/* FIXME */
			break;
		case TT_UNKNOWN:
			fprintf(stderr, "%s%s%s", "configure: ", target,
					": unknown type for target\n");
			return 1;
	}
	return 0;
}

static int _objs_source(Prefs * prefs, FILE * fp, String * source);
static int _target_objs(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	int ret = 0;
	String * sources;
	int i;
	char c;

	if((sources = config_get(config, target, "sources")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", target,
				": no sources defined for target\n");
		return 1;
	}
	if(!(*prefs & PREFS_n))
		fprintf(fp, "%s%s%s", "\n", target, "_OBJS =");
	for(i = 0; ret == 0; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		ret = _objs_source(prefs, fp, sources);
		if(c == '\0')
			break;
		sources[i] = c;
		sources+=i+1;
		i = 0;
	}
	if(!(*prefs & PREFS_n))
		fputc('\n', fp);
	return ret;
}

static int _objs_source(Prefs * prefs, FILE * fp, String * source)
{
	int ret = 0;
	String * extension;
	ObjectType ot;
	int len;

	if((extension = _source_extension(source)) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", source,
				": no extension for source\n");
		return 1;
	}
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	ot = _object_type(extension);
	switch(ot)
	{
		case OT_ASM_SOURCE:
		case OT_C_SOURCE:
			if(*prefs & PREFS_n)
				break;
			fprintf(fp, "%s%s%s", " ", source, ".o");
			break;
		case OT_UNKNOWN:
			ret = 1;
			fprintf(stderr, "%s%s%s", "configure: ", source,
					": unknown extension for source\n");
			break;
	}
	source[len] = '.';
	return ret;
}

static int _target_library(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	String * p;

	if(_target_objs(prefs, config, fp, target) != 0)
		return 1;
	if(*prefs & PREFS_n)
		return 0;
	fprintf(fp, "%s%s", target, "_CFLAGS = $(CFLAGSF) $(CFLAGS)");
	if((p = config_get(config, target, "cflags")) != NULL)
		fprintf(fp, "%s%s", " ", p);
	fputc('\n', fp);
	fprintf(fp, "%s%s%s%s", target, ".a: $(", target, "_OBJS)\n");
	fprintf(fp, "%s%s%s%s%s", "\t$(AR) ", target, ".a $(", target,
			"_OBJS)\n");
	fprintf(fp, "%s%s%s", "\t$(RANLIB) ", target, ".a\n");
	fprintf(fp, "%s%s%s%s", target, ".so: $(", target, "_OBJS)\n");
	fprintf(fp, "%s%s%s%s%s", "\t$(LD) -o ", target, ".so $(", target,
			"_OBJS)\n");
	return 0;
}

static int _objects_target(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _write_objects(Prefs * prefs, Config * config, FILE * fp)
{
	char * targets = config_get(config, "", "targets");
	char c;
	int i;
	int ret = 0;

	if(targets == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		ret += _objects_target(prefs, config, fp, targets);
		if(c == '\0')
			break;
		targets[i] = c;
		targets+=i+1;
		i = 0;
	}
	return ret;
}

static int _target_source(Prefs * prefs, FILE * fp, String * target,
		String * source);
static int _objects_target(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	String * sources;
	int i;
	char c;

	if((sources = config_get(config, target, "sources")) == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		_target_source(prefs, fp, target, sources);
		if(c == '\0')
			break;
		sources[i] = c;
		sources+=i+1;
		i = 0;
	}
	return 0;
}

static int _target_source(Prefs * prefs, FILE * fp, String * target,
		String * source)
{
	int ret = 0;
	String * extension;
	ObjectType ot;
	int len;

	if((extension = _source_extension(source)) == NULL)
		return 1;
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	ot = _object_type(extension);
	switch(ot)
	{
		case OT_ASM_SOURCE:
		case OT_C_SOURCE:
			if(*prefs & PREFS_n)
				break;
			fprintf(fp, "%s%s%s%s%s%s%s", "\n", source, ".o: ",
					source, ".", sObjectType[ot], "\n");
			fprintf(fp, "%s%s%s%s%s%s", "\t$(CC) $(", target,
					"_CFLAGS) -c ", source, ".",
					sObjectType[ot]);
			if(string_find(source, "/"))
				fprintf(fp, "%s%s%s", " -o ", source, ".o");
			fputc('\n', fp);
			break;
		case OT_UNKNOWN:
			ret = 1;
			break;
	}
	source[len] = '.';
	return ret;
}

static int _clean_targets(Config * config, FILE * fp);
static int _write_clean(Prefs * prefs, Config * config, FILE * fp)
{
	if(*prefs & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\nclean:\n");
	if(config_get(config, "", "subdirs") != NULL)
		fprintf(fp, "%s", "\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE) clean) || exit; done\n");
	return _clean_targets(config, fp);
}

static int _clean_targets(Config * config, FILE * fp)
{
	String * targets;
	int i;
	char c;

	if((targets = config_get(config, "", "targets")) == NULL)
		return 0;
	fprintf(fp, "%s", "\t$(RM)");
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		fprintf(fp, "%s%s%s", " $(", targets, "_OBJS)");
		if(c == '\0')
			break;
		targets[i] = c;
		targets+=i+1;
		i = 0;
	}
	fputc('\n', fp);
	return 0;
}

static int _write_distclean(Prefs * prefs, Config * config, FILE * fp)
{
	String * subdirs;

	if(*prefs & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\ndistclean:");
	if((subdirs = config_get(config, "", "subdirs")) == NULL)
		fprintf(fp, "%s", " clean\n");
	else
	{
		fprintf(fp, "%s", "\n\t@for i in $(SUBDIRS); do (cd $$i"
				" && $(MAKE) distclean) || exit; done\n");
		_clean_targets(config, fp);
	}
	if(config_get(config, "", "targets") != NULL)
		fprintf(fp, "%s", "\t$(RM) $(TARGETS)\n");
	return 0;
}

static int _dist_subdir(Config * config, FILE * fp, Config * subdir);
static int _write_dist(Prefs * prefs, Config * config, FILE * fp,
		configArray * ca, int from, int to)
{
	String const * package;
	String const * version;
	Config * p;
	int i;

	if(*prefs & PREFS_n)
		return 0;
	if((package = config_get(config, "", "package")) == NULL
			|| (version = config_get(config, "", "version"))
			== NULL)
		return 0;
	fprintf(fp, "%s%s", "\ndist: distclean\n",
			"\t@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \\\n");
	for(i = to; i > from; i--)
	{
		array_get_copy(ca, i - 1, &p);
		_dist_subdir(config, fp, p);
	}
	return 0;
}

static int _dist_subdir_dist(FILE * fp, String * path, String * dist);
static int _dist_subdir(Config * config, FILE * fp, Config * subdir)
{
	String * path;
	int len;
	String * targets;
	String * dist;
	int i;
	char c;

	path = config_get(config, "", "directory");
	len = string_length(path);
	path = config_get(subdir, "", "directory");
	path = &path[len];
	if(path[0] == '/')
		path++;
	if((targets = config_get(subdir, "", "targets")) != NULL)
	{
		/* FIXME unique SOURCES */
		for(i = 0;; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			if((dist = config_get(subdir, targets, "sources"))
					!= NULL)
				_dist_subdir_dist(fp, path, dist);
			if(c == '\0')
				break;
			targets[i] = c;
			targets+=i+1;
			i = 0;
		}
	}
	if((dist = config_get(subdir, "", "dist")) != NULL)
		_dist_subdir_dist(fp, path, dist);
	fprintf(fp, "%s%s%s%s", "\t\t", path, path[0] == '\0' ? "" : "/",
			PROJECT_CONF " \\\n");
	fprintf(fp, "%s%s%s%s%s", "\t\t", path, path[0] == '\0' ? "" : "/",
			MAKEFILE, path[0] == '\0' ? "\n" : " \\\n");
	return 0;
}

static int _dist_subdir_dist(FILE * fp, String * path, String * dist)
{
	int i;
	char c;

	for(i = 0;; i++)
	{
		if(dist[i] != ',' && dist[i] != '\0')
			continue;
		c = dist[i];
		dist[i] = '\0';
		fprintf(fp, "%s%s%s%s%s", "\t\t", path[0] == '\0' ? "" : path,
				path[0] == '\0' ? "" : "/", dist, " \\\n");
		if(c == '\0')
			break;
		dist[i] = c;
		dist+=i+1;
		i = 0;
	}
	return 0;
}

static int _install_target(Config * config, FILE * fp, String * target);
static int _write_install(Prefs * prefs, Config * config, FILE * fp)
{
	int ret = 0;
	String * subdirs;
	String * targets;
	int i;
	char c;

	if(*prefs & PREFS_n)
		return 0;
	fprintf(fp, "%s", "install: all\n");
	if((subdirs = config_get(config, "", "subdirs")) != NULL)
		fprintf(fp, "%s", "\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE) install) || exit; done\n");
	if((targets = config_get(config, "", "targets")) != NULL)
		for(i = 0; ret == 0; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			ret = _install_target(config, fp, targets);
			if(c == '\0')
				break;
			targets[i] = c;
			targets+=i+1;
			i = 0;
		}
	return ret;
}

static int _install_target(Config * config, FILE * fp, String * target)
{
	String * type;
	static Config * flag = NULL;
	static int done[TT_LAST];
	TargetType tt;

	if((type = config_get(config, target, "type")) == NULL)
		return 1;
	if(flag != config)
	{
		flag = config;
		memset(done, 0, sizeof(done));
	}
	switch((tt = _target_type(type)))
	{
		case TT_BINARY:
			if(!done[tt])
				fprintf(fp, "%s", "\t$(MKDIR) $(DESTDIR)"
						"$(BINDIR)\n");
			fprintf(fp, "%s%s%s%s%s", "\t$(INSTALL) -m 0755 ",
					target, " $(DESTDIR)$(BINDIR)/",
					target, "\n");
			break;
		case TT_LIBRARY:
			if(!done[tt])
				fprintf(fp, "%s", "\t$(MKDIR) $(DESTDIR)"
						"$(LIBDIR)\n");
			fprintf(fp, "%s%s%s%s%s", "\t$(INSTALL) -m 0644 ",
					target, ".a $(DESTDIR)$(LIBDIR)/",
					target, ".a\n");
			fprintf(fp, "%s%s%s%s%s", "\t$(INSTALL) -m 0755 ",
					target, ".so $(DESTDIR)$(LIBDIR)/",
					target, ".so\n");
			break;
		case TT_OBJECT:
		case TT_UNKNOWN:
			break;
	}
	done[tt] = 1;
	return 0;
}

/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: configure [-nv][directory]\n\
  -n	do not actually write Makefiles\n\
  -v	verbose mode\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	int o;

	while((o = getopt(argc, argv, "nv")) != -1)
		switch(o)
		{
			case 'n':
				prefs |= PREFS_n;
				break;
			case 'v':
				prefs |= PREFS_v;
				break;
			case '?':
				return _usage();
		}
	if(argc - optind > 1)
		return _usage();
	return _configure(&prefs, argc - optind == 1 ? argv[argc - 1] : ".");
}
