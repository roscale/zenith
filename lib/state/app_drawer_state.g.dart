// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'app_drawer_state.dart';

// **************************************************************************
// RiverpodGenerator
// **************************************************************************

// ignore_for_file: avoid_private_typedef_functions, non_constant_identifier_names, subtype_of_sealed_class, invalid_use_of_internal_member, unused_element, constant_identifier_names, unnecessary_raw_strings, library_private_types_in_public_api

/// Copied from Dart SDK
class _SystemHash {
  _SystemHash._();

  static int combine(int hash, int value) {
    // ignore: parameter_assignments
    hash = 0x1fffffff & (hash + value);
    // ignore: parameter_assignments
    hash = 0x1fffffff & (hash + ((0x0007ffff & hash) << 10));
    return hash ^ (hash >> 6);
  }

  static int finish(int hash) {
    // ignore: parameter_assignments
    hash = 0x1fffffff & (hash + ((0x03ffffff & hash) << 3));
    // ignore: parameter_assignments
    hash = hash ^ (hash >> 11);
    return 0x1fffffff & (hash + ((0x00003fff & hash) << 15));
  }
}

String $fileToScalableImageHash() =>
    r'cdee05cbedc6229524232afe652f39fafdb7da6f';

/// See also [fileToScalableImage].
class FileToScalableImageProvider extends FutureProvider<ScalableImage> {
  FileToScalableImageProvider(
    this.path,
  ) : super(
          (ref) => fileToScalableImage(
            ref,
            path,
          ),
          from: fileToScalableImageProvider,
          name: r'fileToScalableImageProvider',
          debugGetCreateSourceHash:
              const bool.fromEnvironment('dart.vm.product')
                  ? null
                  : $fileToScalableImageHash,
        );

  final String path;

  @override
  bool operator ==(Object other) {
    return other is FileToScalableImageProvider && other.path == path;
  }

  @override
  int get hashCode {
    var hash = _SystemHash.combine(0, runtimeType.hashCode);
    hash = _SystemHash.combine(hash, path.hashCode);

    return _SystemHash.finish(hash);
  }
}

typedef FileToScalableImageRef = FutureProviderRef<ScalableImage>;

/// See also [fileToScalableImage].
final fileToScalableImageProvider = FileToScalableImageFamily();

class FileToScalableImageFamily extends Family<AsyncValue<ScalableImage>> {
  FileToScalableImageFamily();

  FileToScalableImageProvider call(
    String path,
  ) {
    return FileToScalableImageProvider(
      path,
    );
  }

  @override
  FutureProvider<ScalableImage> getProviderOverride(
    covariant FileToScalableImageProvider provider,
  ) {
    return call(
      provider.path,
    );
  }

  @override
  List<ProviderOrFamily>? get allTransitiveDependencies => null;

  @override
  List<ProviderOrFamily>? get dependencies => null;

  @override
  String? get name => r'fileToScalableImageProvider';
}

String $desktopEntriesHash() => r'b928031c1c972ae7dd1dc4ebba5f4ba46c41ec35';

/// See also [desktopEntries].
final desktopEntriesProvider = FutureProvider<List<LocalizedDesktopEntry>>(
  desktopEntries,
  name: r'desktopEntriesProvider',
  debugGetCreateSourceHash: const bool.fromEnvironment('dart.vm.product')
      ? null
      : $desktopEntriesHash,
);
typedef DesktopEntriesRef = FutureProviderRef<List<LocalizedDesktopEntry>>;
String $defaultIconThemeHash() => r'79007c08190c70b87508641a1b92a2aaffdfd96c';

/// See also [defaultIconTheme].
final defaultIconThemeProvider = FutureProvider<dynamic>(
  defaultIconTheme,
  name: r'defaultIconThemeProvider',
  debugGetCreateSourceHash: const bool.fromEnvironment('dart.vm.product')
      ? null
      : $defaultIconThemeHash,
);
typedef DefaultIconThemeRef = FutureProviderRef<dynamic>;
