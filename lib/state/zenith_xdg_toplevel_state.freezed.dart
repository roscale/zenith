// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target

part of 'zenith_xdg_toplevel_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$ZenithXdgToplevelState {
  bool get visible => throw _privateConstructorUsedError;
  Key get virtualKeyboardKey => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $ZenithXdgToplevelStateCopyWith<ZenithXdgToplevelState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $ZenithXdgToplevelStateCopyWith<$Res> {
  factory $ZenithXdgToplevelStateCopyWith(ZenithXdgToplevelState value,
          $Res Function(ZenithXdgToplevelState) then) =
      _$ZenithXdgToplevelStateCopyWithImpl<$Res, ZenithXdgToplevelState>;
  @useResult
  $Res call({bool visible, Key virtualKeyboardKey});
}

/// @nodoc
class _$ZenithXdgToplevelStateCopyWithImpl<$Res,
        $Val extends ZenithXdgToplevelState>
    implements $ZenithXdgToplevelStateCopyWith<$Res> {
  _$ZenithXdgToplevelStateCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? visible = null,
    Object? virtualKeyboardKey = null,
  }) {
    return _then(_value.copyWith(
      visible: null == visible
          ? _value.visible
          : visible // ignore: cast_nullable_to_non_nullable
              as bool,
      virtualKeyboardKey: null == virtualKeyboardKey
          ? _value.virtualKeyboardKey
          : virtualKeyboardKey // ignore: cast_nullable_to_non_nullable
              as Key,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_ZenithXdgToplevelStateCopyWith<$Res>
    implements $ZenithXdgToplevelStateCopyWith<$Res> {
  factory _$$_ZenithXdgToplevelStateCopyWith(_$_ZenithXdgToplevelState value,
          $Res Function(_$_ZenithXdgToplevelState) then) =
      __$$_ZenithXdgToplevelStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call({bool visible, Key virtualKeyboardKey});
}

/// @nodoc
class __$$_ZenithXdgToplevelStateCopyWithImpl<$Res>
    extends _$ZenithXdgToplevelStateCopyWithImpl<$Res,
        _$_ZenithXdgToplevelState>
    implements _$$_ZenithXdgToplevelStateCopyWith<$Res> {
  __$$_ZenithXdgToplevelStateCopyWithImpl(_$_ZenithXdgToplevelState _value,
      $Res Function(_$_ZenithXdgToplevelState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? visible = null,
    Object? virtualKeyboardKey = null,
  }) {
    return _then(_$_ZenithXdgToplevelState(
      visible: null == visible
          ? _value.visible
          : visible // ignore: cast_nullable_to_non_nullable
              as bool,
      virtualKeyboardKey: null == virtualKeyboardKey
          ? _value.virtualKeyboardKey
          : virtualKeyboardKey // ignore: cast_nullable_to_non_nullable
              as Key,
    ));
  }
}

/// @nodoc

class _$_ZenithXdgToplevelState implements _ZenithXdgToplevelState {
  const _$_ZenithXdgToplevelState(
      {required this.visible, required this.virtualKeyboardKey});

  @override
  final bool visible;
  @override
  final Key virtualKeyboardKey;

  @override
  String toString() {
    return 'ZenithXdgToplevelState(visible: $visible, virtualKeyboardKey: $virtualKeyboardKey)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_ZenithXdgToplevelState &&
            (identical(other.visible, visible) || other.visible == visible) &&
            (identical(other.virtualKeyboardKey, virtualKeyboardKey) ||
                other.virtualKeyboardKey == virtualKeyboardKey));
  }

  @override
  int get hashCode => Object.hash(runtimeType, visible, virtualKeyboardKey);

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_ZenithXdgToplevelStateCopyWith<_$_ZenithXdgToplevelState> get copyWith =>
      __$$_ZenithXdgToplevelStateCopyWithImpl<_$_ZenithXdgToplevelState>(
          this, _$identity);
}

abstract class _ZenithXdgToplevelState implements ZenithXdgToplevelState {
  const factory _ZenithXdgToplevelState(
      {required final bool visible,
      required final Key virtualKeyboardKey}) = _$_ZenithXdgToplevelState;

  @override
  bool get visible;
  @override
  Key get virtualKeyboardKey;
  @override
  @JsonKey(ignore: true)
  _$$_ZenithXdgToplevelStateCopyWith<_$_ZenithXdgToplevelState> get copyWith =>
      throw _privateConstructorUsedError;
}
