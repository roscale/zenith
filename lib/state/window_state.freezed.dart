// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target

part of 'window_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$WindowState {
  bool get visible => throw _privateConstructorUsedError;
  Key get virtualKeyboardKey => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $WindowStateCopyWith<WindowState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $WindowStateCopyWith<$Res> {
  factory $WindowStateCopyWith(
          WindowState value, $Res Function(WindowState) then) =
      _$WindowStateCopyWithImpl<$Res, WindowState>;
  @useResult
  $Res call({bool visible, Key virtualKeyboardKey});
}

/// @nodoc
class _$WindowStateCopyWithImpl<$Res, $Val extends WindowState>
    implements $WindowStateCopyWith<$Res> {
  _$WindowStateCopyWithImpl(this._value, this._then);

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
abstract class _$$_WindowStateCopyWith<$Res>
    implements $WindowStateCopyWith<$Res> {
  factory _$$_WindowStateCopyWith(
          _$_WindowState value, $Res Function(_$_WindowState) then) =
      __$$_WindowStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call({bool visible, Key virtualKeyboardKey});
}

/// @nodoc
class __$$_WindowStateCopyWithImpl<$Res>
    extends _$WindowStateCopyWithImpl<$Res, _$_WindowState>
    implements _$$_WindowStateCopyWith<$Res> {
  __$$_WindowStateCopyWithImpl(
      _$_WindowState _value, $Res Function(_$_WindowState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? visible = null,
    Object? virtualKeyboardKey = null,
  }) {
    return _then(_$_WindowState(
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

class _$_WindowState implements _WindowState {
  const _$_WindowState(
      {required this.visible, required this.virtualKeyboardKey});

  @override
  final bool visible;
  @override
  final Key virtualKeyboardKey;

  @override
  String toString() {
    return 'WindowState(visible: $visible, virtualKeyboardKey: $virtualKeyboardKey)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_WindowState &&
            (identical(other.visible, visible) || other.visible == visible) &&
            (identical(other.virtualKeyboardKey, virtualKeyboardKey) ||
                other.virtualKeyboardKey == virtualKeyboardKey));
  }

  @override
  int get hashCode => Object.hash(runtimeType, visible, virtualKeyboardKey);

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_WindowStateCopyWith<_$_WindowState> get copyWith =>
      __$$_WindowStateCopyWithImpl<_$_WindowState>(this, _$identity);
}

abstract class _WindowState implements WindowState {
  const factory _WindowState(
      {required final bool visible,
      required final Key virtualKeyboardKey}) = _$_WindowState;

  @override
  bool get visible;
  @override
  Key get virtualKeyboardKey;
  @override
  @JsonKey(ignore: true)
  _$$_WindowStateCopyWith<_$_WindowState> get copyWith =>
      throw _privateConstructorUsedError;
}
