// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target

part of 'screen_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$ScreenState {
  bool get on => throw _privateConstructorUsedError;
  bool get pending => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $ScreenStateCopyWith<ScreenState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $ScreenStateCopyWith<$Res> {
  factory $ScreenStateCopyWith(
          ScreenState value, $Res Function(ScreenState) then) =
      _$ScreenStateCopyWithImpl<$Res, ScreenState>;
  @useResult
  $Res call({bool on, bool pending});
}

/// @nodoc
class _$ScreenStateCopyWithImpl<$Res, $Val extends ScreenState>
    implements $ScreenStateCopyWith<$Res> {
  _$ScreenStateCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? on = null,
    Object? pending = null,
  }) {
    return _then(_value.copyWith(
      on: null == on
          ? _value.on
          : on // ignore: cast_nullable_to_non_nullable
              as bool,
      pending: null == pending
          ? _value.pending
          : pending // ignore: cast_nullable_to_non_nullable
              as bool,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_ScreenStateCopyWith<$Res>
    implements $ScreenStateCopyWith<$Res> {
  factory _$$_ScreenStateCopyWith(
          _$_ScreenState value, $Res Function(_$_ScreenState) then) =
      __$$_ScreenStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call({bool on, bool pending});
}

/// @nodoc
class __$$_ScreenStateCopyWithImpl<$Res>
    extends _$ScreenStateCopyWithImpl<$Res, _$_ScreenState>
    implements _$$_ScreenStateCopyWith<$Res> {
  __$$_ScreenStateCopyWithImpl(
      _$_ScreenState _value, $Res Function(_$_ScreenState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? on = null,
    Object? pending = null,
  }) {
    return _then(_$_ScreenState(
      on: null == on
          ? _value.on
          : on // ignore: cast_nullable_to_non_nullable
              as bool,
      pending: null == pending
          ? _value.pending
          : pending // ignore: cast_nullable_to_non_nullable
              as bool,
    ));
  }
}

/// @nodoc

class _$_ScreenState implements _ScreenState {
  const _$_ScreenState({required this.on, required this.pending});

  @override
  final bool on;
  @override
  final bool pending;

  @override
  String toString() {
    return 'ScreenState(on: $on, pending: $pending)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_ScreenState &&
            (identical(other.on, on) || other.on == on) &&
            (identical(other.pending, pending) || other.pending == pending));
  }

  @override
  int get hashCode => Object.hash(runtimeType, on, pending);

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_ScreenStateCopyWith<_$_ScreenState> get copyWith =>
      __$$_ScreenStateCopyWithImpl<_$_ScreenState>(this, _$identity);
}

abstract class _ScreenState implements ScreenState {
  const factory _ScreenState(
      {required final bool on, required final bool pending}) = _$_ScreenState;

  @override
  bool get on;
  @override
  bool get pending;
  @override
  @JsonKey(ignore: true)
  _$$_ScreenStateCopyWith<_$_ScreenState> get copyWith =>
      throw _privateConstructorUsedError;
}
