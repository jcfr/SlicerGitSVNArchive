import glob
import os
import re

from .ExtensionProject import ExtensionProject
from .Utilities import detectEncoding

#=============================================================================
class ExtensionDescription(object):
  """Representation of an extension description.

  This class provides a Python object representation of an extension
  description. The extension information is made available as attributes on the
  object. The "well known" attributes are described
  :wikidoc:`Developers/Extensions/DescriptionFile here`. Custom attributes may
  be added with :func:`setattr`. Attributes may be removed with :func:`delattr`
  or the :meth:`.clear` method.
  """

  _reParam = re.compile(r"([a-zA-Z][a-zA-Z0-9_]*)\s+(.+)")

  #---------------------------------------------------------------------------
  def __init__(self, repo=None, filepath=None, sourcedir=None, encoding=None):
    """
    :param repo:
      Extension repository from which to create the description.
    :type repo:
      :class:`git.Repo <git:git.repo.base.Repo>`,
      :class:`.Subversion.Repository` or ``None``.
    :param filepath:
      Path to an existing ``.s4ext`` to read.
    :type filepath:
      :class:`basestring` or ``None``.
    :param sourcedir:
      Path to an extension source directory.
    :type sourcedir:
      :class:`basestring` or ``None``.
    :param encoding:
      Encoding of the extension description file.
    :type encoding:
      :class:`basestring` or ``None``.

    If ``encoding`` is ``None``, the encoding will be guessed using
    :meth:`~SlicerWizard.Utilities.detectEncoding`.

    :raises:
      * :exc:`~exceptions.KeyError` if the extension description is missing a
        required attribute.
      * :exc:`~exceptions.Exception` if there is some other problem
        constructing the description.

    The description may be created from a repository instance (in which case
    the description repository information will be populated), a path to the
    extension source directory, or a path to an existing ``.s4ext`` file.
    No more than one of ``repo``, ``filepath`` or ``sourcedir`` may be given.
    If none are provided, the description will be incomplete.
    """

    args = (repo, filepath, sourcedir)
    if args.count(None) < len(args) - 1:
      raise Exception("cannot construct %s: only one of"
                      " (repo, filepath, sourcedir) may be given" %
                      type(self).__name__)

    if filepath is not None:
      self.encoding = self._readFile(filepath, encoding)

    elif repo is not None:
      # Handle git repositories
      if hasattr(repo, "remotes"):
        remote = None
        svnRemote = None

        # Get SHA of HEAD (may not exist if no commit has been made yet!)
        try:
          sha = repo.head.commit.hexsha

        except ValueError:
          sha = "NA"

        # Try to get git remote
        try:
          remote = repo.remotes.origin
        except:
          if len(repo.remotes) == 1:
            remote = repo.remotes[0]

        if remote is None:
          # Try to get svn remote
          config = repo.config_reader()
          for s in config.sections():
            if s.startswith("svn-remote"):
              svnRemote = s[12:-1]
              break

          if svnRemote is None:
            # Do we have any remotes?
            if len(repo.remotes) == 0:
              setattr(self, "scm", "git")
              setattr(self, "scmurl", "NA")
              setattr(self, "scmrevision", sha)

            else:
              raise Exception("unable to determine repository's primary remote")

          else:
            si = self._gitSvnInfo(repo, svnRemote)
            setattr(self, "scm", "svn")
            setattr(self, "scmurl", si["URL"])
            setattr(self, "scmrevision", si["Revision"])

        else:
          setattr(self, "scm", "git")
          setattr(self, "scmurl", self._remotePublicUrl(remote))
          setattr(self, "scmrevision", sha)

        sourcedir = repo.working_tree_dir

      # Handle svn repositories
      elif hasattr(repo, "wc_root"):
        setattr(self, "scm", "svn")
        setattr(self, "scmurl", repo.url)
        setattr(self, "scmrevision", repo.last_change_revision)
        sourcedir = repo.wc_root

    else:
      setattr(self, "scm", "local")
      setattr(self, "scmurl", "NA")
      setattr(self, "scmrevision", "NA")

    if sourcedir is not None:
      p = ExtensionProject(sourcedir, encoding=encoding)
      self._setProjectAttribute("homepage", p, required=True)
      self._setProjectAttribute("category", p, required=True)
      self._setProjectAttribute("description", p)
      self._setProjectAttribute("contributors", p)

      self._setProjectAttribute("status", p)
      self._setProjectAttribute("enabled", p, default="1")
      self._setProjectAttribute("depends", p, default="NA")
      self._setProjectAttribute("build_subdirectory", p, default=".")

      self._setProjectAttribute("iconurl", p)
      self._setProjectAttribute("screenshoturls", p)

      if self.scm == "svn":
        self._setProjectAttribute("svnusername", p, elideempty=True)
        self._setProjectAttribute("svnpassword", p, elideempty=True)

      self._encoding = p.encoding

  #---------------------------------------------------------------------------
  def __repr__(self):
    return repr(self.__dict__)

  #---------------------------------------------------------------------------
  @property
  def encoding(self):
    """Character encoding of the extension description file.

    :type: :class:`str` or ``None``

    This provides the character encoding of the description file from which
    the description instance was created. If the encoding cannot be determined,
    the property will have the value ``None``.

    .. 'note' directive needs '\' to span multiple lines!
    .. note:: If ``encoding`` is ``None``, the description information is \
              stored as raw bytes using :class:`str`. In such case, passing a \
              non-ASCII :class:`unicode` to  any method or property \
              assignment that modifies the description may  make it \
              impossible to write the description file back to disk.
    """

    return self._encoding

  #---------------------------------------------------------------------------
  @staticmethod
  def _remotePublicUrl(remote):
    url = remote.url
    if url.startswith("git@"):
      return url.replace(":", "/").replace("git@", "git://")

    return url

  #---------------------------------------------------------------------------
  @staticmethod
  def _gitSvnInfo(repo, remote):
    result = {}
    for l in repo.git.svn('info', R=remote).split("\n"):
      if len(l):
        key, value = l.split(":", 1)
        result[key] = value.strip()
    return result

  #---------------------------------------------------------------------------
  def _setProjectAttribute(self, name, project, default=None, required=False,
                           elideempty=False):

    if default is None and not required:
      default=""

    v = project.getValue("EXTENSION_" + name.upper(), default)

    if len(v) or not elideempty:
      setattr(self, name, v)

  #---------------------------------------------------------------------------
  def clear(self, attr=None):
    """Remove attributes from the extension description.

    :param attr: Name of attribute to remove.
    :type attr: :class:`str` or ``None``

    If ``attr`` is not ``None``, this removes the specified attribute from the
    description object, equivalent to calling ``delattr(instance, attr)``. If
    ``attr`` is ``None``, all attributes are removed.
    """

    for key in self.__dict__.keys() if attr is None else (attr,):
      delattr(self, key)

  #---------------------------------------------------------------------------
  def _readFile(self, filepath, encoding):

    with open(filepath) as fp:
      contents = fp.read()

      if encoding is None:
        encoding, confidence = detectEncoding(contents)

        if encoding is not None:
          if confidence < 0.5:
            logging.warning("%s: encoding detection confidence is %f:"
                            " description file contents might be corrupt" %
                            (filepath, confidence))

      if encoding is not None:
        # If unable to determine encoding, skip unicode conversion... users
        # must not feed any unicode into the script or things will likely break
        # later (e.g. when trying to save the description file)
        pass
      else:
        # Otherwise, decode the contents into unicode
        contents = contents.decode(encoding)

      for l in contents.splitlines():
        m = self._reParam.match(l)
        if m is not None:
          setattr(self, m.group(1), m.group(2).strip())

    return encoding

  #---------------------------------------------------------------------------
  def read(self, path, encoding=None):
    """Read extension description from directory.

    :param path: Directory containing extension description.
    :type path: :class:`basestring`

    :raises:
      :exc:`~exceptions.IOError` if ``path`` does not contain exactly one
      extension description file.

    This attempts to read an extension description from the specified ``path``
    which contains a single extension description (``.s4ext``) file (usually an
    extension build directory).
    """

    self.clear()

    descriptionFiles = glob.glob(os.path.join(path, "*.[Ss]4[Ee][Xx][Tt]"))
    if len(descriptionFiles) < 1:
      raise IOError("extension description file not found")

    if len(descriptionFiles) > 1:
      raise IOError("multiple extension description files found")

    self.encoding = self._readFile(descriptionFiles[0], encoding)

  #---------------------------------------------------------------------------
  def _write(self, fp, encoding):

    if encoding is None:
      encoding = self._encoding

    if encoding is None and self.encoding is not None:
      encoding = self.encoding if self.encoding.lower() != "ascii" else "utf-8"

    for key in sorted(self.__dict__):
      if key.startswith("_"):
        continue
      content = (u"%s %s" % (key, getattr(self, key))).strip() + "\n"

      if encoding is None:
        # If no encoding is specified and we don't know the original encoding,
        # perform no conversion and hope for the best (will only work if there
        # are no unicode instances in the script)
        fp.write(str(content))

      else:
        # Otherwise, write the file using full encoding conversion
        fp.write(content.encode(encoding))

  #---------------------------------------------------------------------------
  def write(self, out, encoding=None):
    """Write extension description to a file or stream.

    :param out: Stream or path to which to write the description.
    :type out: :class:`~io.IOBase` or :class:`basestring`

    This writes the extension description to the specified file path or stream
    object. This is suitable for producing a ``.s4ext`` file from a description
    object.
    """

    if hasattr(out, "write") and callable(out.write):
      self._write(out, encoding)

    else:
      with open(out, "w") as fp:
        self._write(fp, encoding)
