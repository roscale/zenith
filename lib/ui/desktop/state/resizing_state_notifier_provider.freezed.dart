// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target, unnecessary_question_mark

part of 'resizing_state_notifier_provider.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$ResizerState {
  ResizingSide? get resizingSide => throw _privateConstructorUsedError;
  Size get wantedSize => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $ResizerStateCopyWith<ResizerState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $ResizerStateCopyWith<$Res> {
  factory $ResizerStateCopyWith(
          ResizerState value, $Res Function(ResizerState) then) =
      _$ResizerStateCopyWithImpl<$Res, ResizerState>;
  @useResult
  $Res call({ResizingSide? resizingSide, Size wantedSize});
}

/// @nodoc
class _$ResizerStateCopyWithImpl<$Res, $Val extends ResizerState>
    implements $ResizerStateCopyWith<$Res> {
  _$ResizerStateCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? resizingSide = freezed,
    Object? wantedSize = null,
  }) {
    return _then(_value.copyWith(
      resizingSide: freezed == resizingSide
          ? _value.resizingSide
          : resizingSide // ignore: cast_nullable_to_non_nullable
              as ResizingSide?,
      wantedSize: null == wantedSize
          ? _value.wantedSize
          : wantedSize // ignore: cast_nullable_to_non_nullable
              as Size,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_ResizerStateCopyWith<$Res>
    implements $ResizerStateCopyWith<$Res> {
  factory _$$_ResizerStateCopyWith(
          _$_ResizerState value, $Res Function(_$_ResizerState) then) =
      __$$_ResizerStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call({ResizingSide? resizingSide, Size wantedSize});
}

/// @nodoc
class __$$_ResizerStateCopyWithImpl<$Res>
    extends _$ResizerStateCopyWithImpl<$Res, _$_ResizerState>
    implements _$$_ResizerStateCopyWith<$Res> {
  __$$_ResizerStateCopyWithImpl(
      _$_ResizerState _value, $Res Function(_$_ResizerState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? resizingSide = freezed,
    Object? wantedSize = null,
  }) {
    return _then(_$_ResizerState(
      resizingSide: freezed == resizingSide
          ? _value.resizingSide
          : resizingSide // ignore: cast_nullable_to_non_nullable
              as ResizingSide?,
      wantedSize: null == wantedSize
          ? _value.wantedSize
          : wantedSize // ignore: cast_nullable_to_non_nullable
              as Size,
    ));
  }
}

/// @nodoc

class _$_ResizerState implements _ResizerState {
  const _$_ResizerState({required this.resizingSide, required this.wantedSize});

  @override
  final ResizingSide? resizingSide;
  @override
  final Size wantedSize;

  @override
  String toString() {
    return 'ResizerState(resizingSide: $resizingSide, wantedSize: $wantedSize)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_ResizerState &&
            (identical(other.resizingSide, resizingSide) ||
                other.resizingSide == resizingSide) &&
            (identical(other.wantedSize, wantedSize) ||
                other.wantedSize == wantedSize));
  }

  @override
  int get hashCode => Object.hash(runtimeType, resizingSide, wantedSize);

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_ResizerStateCopyWith<_$_ResizerState> get copyWith =>
      __$$_ResizerStateCopyWithImpl<_$_ResizerState>(this, _$identity);
}

abstract class _ResizerState implements ResizerState {
  const factory _ResizerState(
      {required final ResizingSide? resizingSide,
      required final Size wantedSize}) = _$_ResizerState;

  @override
  ResizingSide? get resizingSide;
  @override
  Size get wantedSize;
  @override
  @JsonKey(ignore: true)
  _$$_ResizerStateCopyWith<_$_ResizerState> get copyWith =>
      throw _privateConstructorUsedError;
}
