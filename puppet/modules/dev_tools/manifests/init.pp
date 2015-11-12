class dev_tools {

  # Update apt before getting any packages (if we need to)
  exec { "apt-update":
    command => "/usr/bin/apt-get update",
    onlyif => "/bin/sh -c '[ ! -f /var/cache/apt/pkgcache.bin ] || /usr/bin/find /etc/apt/* -cnewer /var/cache/apt/pkgcache.bin | /bin/grep . > /dev/null'",
  } ->
  exec { "install-software-properties":
    command => "/usr/bin/apt-get install -y software-properties-common",
    unless => '/usr/bin/dpkg -s software-properties-common',
  } ->
  apt::ppa {'ppa:ubuntu-toolchain-r/test': } ~>
  exec { "apt-update-ppa":
    command => "/usr/bin/apt-get update",
    refreshonly => true
  } -> Package <| |>

  package { 'vim': ensure => latest, }
  package { 'screen': ensure => latest, }
  package { 'htop': ensure => latest, }
  package { 'gdb': ensure => latest, }
  package { 'cmake-curses-gui': ensure => latest, }
  package { 'linux-headers-generic': ensure => latest, }
  package { 'dos2unix': ensure => latest, }
  package { 'unzip': ensure => latest, }
  package { 'rsync': ensure => latest, }
  package { 'git': ensure => latest, }
  package { 'graphviz': ensure => latest, }
  package { 'build-essential': ensure => latest, }
  package { 'python-dev': ensure => latest, }
  package { 'python-pip': ensure => latest, }
  package { 'libncurses5-dev': ensure => latest, }
  package { 'libpcre3-dev': ensure => latest, }
  package { 'gcc-5': ensure => latest, require => Apt::Ppa['ppa:ubuntu-toolchain-r/test'] }
  package { 'g++-5': ensure => latest, require => Apt::Ppa['ppa:ubuntu-toolchain-r/test'] }
  package { 'gfortran-5': ensure => latest, require => Apt::Ppa['ppa:ubuntu-toolchain-r/test'] }
  package { 'binutils': ensure => latest, require => Apt::Ppa['ppa:ubuntu-toolchain-r/test'] }
  package { 'binutils-dev': ensure => latest, require => Apt::Ppa['ppa:ubuntu-toolchain-r/test'] }
  package { 'ninja-build': ensure => latest, }
  package { 'yasm': ensure => latest, }
  package { 'byacc': ensure => latest, }

  # INSTALL PYTHON PACKAGES (we need python-pip to use the pip provider)
  Package['python-pip'] -> Package <| provider == 'pip' |>
  package { 'pyparsing': ensure => latest, provider => 'pip' }
  package { 'pydotplus': ensure => latest, provider => 'pip' }
  package { 'pygments': ensure => latest, provider => 'pip' }
  # python::pip { 'pybfd': ensure => latest }#, url => 'https://github.com/Groundworkstech/pybfd/archive/master.tar.gz' }

  # SSH KEYS FOR THE VM
  file { 'vm_private_key':
      path => '/home/vagrant/.ssh/id_rsa',
      ensure => present,
      source => 'puppet:///modules/dev_tools/id_rsa',
      owner => 'vagrant',
      mode => '600', }

  file { 'vm_public_key':
      path => '/home/vagrant/.ssh/id_rsa.pub',
      ensure => present,
      source => 'puppet:///modules/dev_tools/id_rsa.pub',
      owner => 'vagrant', }

  # SSH CONFIG FOR THE VM
  file { 'ssh_config':
      path => '/home/vagrant/.ssh/config',
      ensure => present,
      source => 'puppet:///modules/dev_tools/ssh_config',
      owner => 'vagrant',
      mode => '600', }

  # SETUP ENVIRONMENT VARIABLES FOR SHELLS
  file { '/etc/profile.d/toolchain_init.sh':
    ensure => present,
    mode => '755',
    source => 'puppet:///modules/dev_tools/toolchain_init.sh', }

  # SETUP BINUTILS REDIRECTS TO USE PLUGINS
  exec { 'redirect_ar':
    command => 'mv /usr/bin/ar /usr/bin/ar_bin',
    onlyif => 'file /usr/bin/ar | grep -q "ELF"' } ->
  file { '/usr/bin/ar':
    ensure => present,
    mode => '755',
    source => 'puppet:///modules/dev_tools/ar', }

  exec { 'redirect_nm':
    command => 'mv /usr/bin/nm /usr/bin/nm_bin',
    onlyif => 'file /usr/bin/nm | grep -q "ELF"' } ->
  file { '/usr/bin/nm':
    ensure => present,
    mode => '755',
    source => 'puppet:///modules/dev_tools/nm', }

  exec { 'redirect_ranlib':
    command => 'mv /usr/bin/ranlib /usr/bin/ranlib_bin',
    onlyif => 'file /usr/bin/ranlib | grep -q "ELF"' } ->
  file { '/usr/bin/ranlib':
    ensure => present,
    mode => '755',
    source => 'puppet:///modules/dev_tools/ranlib', }

  # SETUP OUR ALTERNATIVES SO WE USE THE CORRECT COMPILER
  exec {'fix_compiler_environment':
    command => 'update-alternatives --install /usr/bin/ld ld /usr/bin/ld.bfd 10 \
             && update-alternatives --install /usr/bin/ld ld /usr/bin/ld.gold 20 \
             && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 100 \
                                    --slave /usr/bin/g++ g++ /usr/bin/g++-5 \
                                    --slave /usr/bin/gfortran gfortran /usr/bin/gfortran-5',
    require => [ Package['gcc-5'], Package['g++-5'], Package['gfortran-5'], Package['build-essential'], Package['binutils'] ]
  }
}
