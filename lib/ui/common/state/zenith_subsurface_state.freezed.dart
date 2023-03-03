// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target, unnecessary_question_mark

part of 'zenith_subsurface_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$ZenithSubsurfaceState {
  Offset get position =>
      throw _privateConstructorUsedError; // relative to the parent
  bool get mapped => throw _privateConstructorUsedError;
  Key get widgetKey => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $ZenithSubsurfaceStateCopyWith<ZenithSubsurfaceState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $ZenithSubsurfaceStateCopyWith<$Res> {
  factory $ZenithSubsurfaceStateCopyWith(ZenithSubsurfaceState value,
          $Res Function(ZenithSubsurfaceState) then) =
      _$ZenithSubsurfaceStateCopyWithImpl<$Res, ZenithSubsurfaceState>;
  @useResult
  $Res call({Offset position, bool mapped, Key widgetKey});
}

/// @nodoc
class _$ZenithSubsurfaceStateCopyWithImpl<$Res,
        $Val extends ZenithSubsurfaceState>
    implements $ZenithSubsurfaceStateCopyWith<$Res> {
  _$ZenithSubsurfaceStateCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? position = null,
    Object? mapped = null,
    Object? widgetKey = null,
  }) {
    return _then(_value.copyWith(
      position: null == position
          ? _value.position
          : position // ignore: cast_nullable_to_non_nullable
              as Offset,
      mapped: null == mapped
          ? _value.mapped
          : mapped // ignore: cast_nullable_to_non_nullable
              as bool,
      widgetKey: null == widgetKey
          ? _value.widgetKey
          : widgetKey // ignore: cast_nullable_to_non_nullable
              as Key,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_ZenithSubsurfaceStateCopyWith<$Res>
    implements $ZenithSubsurfaceStateCopyWith<$Res> {
  factory _$$_ZenithSubsurfaceStateCopyWith(_$_ZenithSubsurfaceState value,
          $Res Function(_$_ZenithSubsurfaceState) then) =
      __$$_ZenithSubsurfaceStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call({Offset position, bool mapped, Key widgetKey});
}

/// @nodoc
class __$$_ZenithSubsurfaceStateCopyWithImpl<$Res>
    extends _$ZenithSubsurfaceStateCopyWithImpl<$Res, _$_ZenithSubsurfaceState>
    implements _$$_ZenithSubsurfaceStateCopyWith<$Res> {
  __$$_ZenithSubsurfaceStateCopyWithImpl(_$_ZenithSubsurfaceState _value,
      $Res Function(_$_ZenithSubsurfaceState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? position = null,
    Object? mapped = null,
    Object? widgetKey = null,
  }) {
    return _then(_$_ZenithSubsurfaceState(
      position: null == position
          ? _value.position
          : position // ignore: cast_nullable_to_non_nullable
              as Offset,
      mapped: null == mapped
          ? _value.mapped
          : mapped // ignore: cast_nullable_to_non_nullable
              as bool,
      widgetKey: null == widgetKey
          ? _value.widgetKey
          : widgetKey // ignore: cast_nullable_to_non_nullable
              as Key,
    ));
  }
}

/// @nodoc

class _$_ZenithSubsurfaceState implements _ZenithSubsurfaceState {
  const _$_ZenithSubsurfaceState(
      {required this.position, required this.mapped, required this.widgetKey});

  @override
  final Offset position;
// relative to the parent
  @override
  final bool mapped;
  @override
  final Key widgetKey;

  @override
  String toString() {
    return 'ZenithSubsurfaceState(position: $position, mapped: $mapped, widgetKey: $widgetKey)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_ZenithSubsurfaceState &&
            (identical(other.position, position) ||
                other.position == position) &&
            (identical(other.mapped, mapped) || other.mapped == mapped) &&
            (identical(other.widgetKey, widgetKey) ||
                other.widgetKey == widgetKey));
  }

  @override
  int get hashCode => Object.hash(runtimeType, position, mapped, widgetKey);

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_ZenithSubsurfaceStateCopyWith<_$_ZenithSubsurfaceState> get copyWith =>
      __$$_ZenithSubsurfaceStateCopyWithImpl<_$_ZenithSubsurfaceState>(
          this, _$identity);
}

abstract class _ZenithSubsurfaceState implements ZenithSubsurfaceState {
  const factory _ZenithSubsurfaceState(
      {required final Offset position,
      required final bool mapped,
      required final Key widgetKey}) = _$_ZenithSubsurfaceState;

  @override
  Offset get position;
  @override // relative to the parent
  bool get mapped;
  @override
  Key get widgetKey;
  @override
  @JsonKey(ignore: true)
  _$$_ZenithSubsurfaceStateCopyWith<_$_ZenithSubsurfaceState> get copyWith =>
      throw _privateConstructorUsedError;
}
