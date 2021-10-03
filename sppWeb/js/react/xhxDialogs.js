'use strict';

const e = React.createElement;

//header can be shared
class dlgHeader extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      visible: true
    };
    this.toggleDialog = this.toggleDialog.bind(this);
  };

  toggleDialog(e) {
    //target: the X button
    //parent: the header
    //parent's parent: the dialog
    //there may be better way to close the dialog
    e.target.parentElement.parentElement.style.display = "none";
  }

  render() {
    return e("div", {className:'dlg_header'}, ` ${this.props.title}`, 
              e('button', {onClick:this.toggleDialog, style:{float:'right'}}, 'X'));
  }
}

class dlgCuttingPlaneContents extends React.Component {
  constructor(props) {
    super(props);
    this.state = { selectedGCutOpt: 'optOneSide', cutHalfWidthScaleFactor: '1.0' };
  }

  handleDistanceChange(e) {
    this.setState ({ cutHalfWidthScaleFactor: e.target.value });
   }

  setTranslationType(event) {
    var edt = editor;
    if (event.target.value == 'r0') {
      editor.signals.xhxProjectSettingsChanged.dispatch(13, {enable:false});//13: enable/disable cutting plane
    } else
    if (event.target.value == 'r1') {
      editor.signals.xhxProjectSettingsChanged.dispatch(13, {enable:true, method:'rotate'});//13: enable/disable cutting plane
    } else
    if (event.target.value == 'r2') {
      editor.signals.xhxProjectSettingsChanged.dispatch(13, {enable:true, method:'translate'});
    }
  } 

  onNewBtnClick(event) {
    if (event.target.name != 'New') 
      return;

      editor.signals.xhxProjectSettingsChanged.dispatch(16, {cmdName:'New'});//16: create/delete cutting plane
  }

  onDeleteBtnClick(event) {
    if (event.target.name != 'Delete') 
      return;
      editor.signals.xhxProjectSettingsChanged.dispatch(16, {cmdName:'Delete'});//16: create/delete cutting plane
  }

  onRotateBtnClick(event) {
    if (event.target.name == 'Flip') {
      var opt = {
        'axis': 'X',
        'angle': 3.1415926
      };
      var s = editor.signals;
      s.xhxProjectSettingsChanged.dispatch(12, opt);//12: flip cutting plane normal
    } else
    if (event.target.name == 'xR90') {
      var opt = {
        'axis': 'X',
        'angle': 3.1415926/2
      };
      var s = editor.signals;
      s.xhxProjectSettingsChanged.dispatch(12, opt);//12: flip cutting plane normal
    } else
    if (event.target.name == 'yR90') {
      var opt = {
        'axis': 'Y',
        'angle': 3.1415926/2
      };
      var s = editor.signals;
      s.xhxProjectSettingsChanged.dispatch(12, opt);//12: flip cutting plane normal
    } else
    if (event.target.name == 'zR90') {
      var opt = {
        'axis': 'Z',
        'angle': 3.1415926/2
      };
      var s = editor.signals;
      s.xhxProjectSettingsChanged.dispatch(12, opt);//12: flip cutting plane normal
    }
  }

  handleGlyphCutOptionChange(changeEvent) {
    this.setState({
      selectedGCutOpt: changeEvent.target.value
    });

    var opt = {
      'onOneSide': changeEvent.target.value === 'optOneSide',
      'halfWidthScaleFactor': this.state.cutHalfWidthScaleFactor
    };

    editor.signals.xhxProjectSettingsChanged.dispatch(14, opt);
  }

  setGlyphOnCutSide(event) {
    if (event.target.value == 'r0') {//on one side
      this.setState ({ glyphsOnOneSide: 'true' });
    } else
    if (event.target.value == 'r1') {//on both sides
      this.setState ({ glyphsOnOneSide: 'false' });
    }

    var opt = {
      'onOneSide': this.state.glyphsOnOneSide,
      'halfWidthScaleFactor': this.state.cutHalfWidthScaleFactor
    };

    var s = editor.signals;
    s.xhxProjectSettingsChanged.dispatch(14, opt);
  } 

  onHideSurface(event) {
    var lSurfaceVisible = !event.target.checked;
    var s = editor.signals;
    s.xhxProjectSettingsChanged.dispatch(11, lSurfaceVisible);//11: part surface display
  } 
 
  onApplyCutWidthScaleFactor(e) {
    var opt = {
      'onOneSide': this.state.selectedGCutOpt === 'optOneSide',
      'halfWidthScaleFactor': this.state.cutHalfWidthScaleFactor
    };
    editor.signals.xhxProjectSettingsChanged.dispatch(14, opt);
  }

  render() {
    const btnStyle = {
      borderRadius: '4px', 
      border: '2px solid #4CAF50', 
      width: '25%'
    };

    return e("div", null, 
              e('button', {style: btnStyle, onClick: this.onNewBtnClick.bind(this), name:'New'}, "New"),
              e('button', {style: btnStyle, onClick: this.onDeleteBtnClick.bind(this), name:'Delete'}, "Delete"),
              e('br', null),
              e('form', null, 
                e('fieldset', {onChange:this.setTranslationType.bind(this)}, e('legend', {style: {textAlign: 'left'}}, "Translation Control:"),
                  e('input', {type:"radio", name:'translateType', value:'r0', defaultChecked:'checked'}), e('label', null, "Hide"),
                  e('input', {type:"radio", name:'translateType', value:'r1'}), e('label', null, "Rotation"),
                  e('input', {type:"radio", name:'translateType', value:'r2'}), e('label', null, "Translation")
               )),
              e('br', null),
              e('button', {style: btnStyle, onClick: this.onRotateBtnClick.bind(this), name:'Flip'}, "Flip"),
              e('button', {style: btnStyle, onClick: this.onRotateBtnClick.bind(this), name:'xR90'}, "xR+90"),
              e('button', {style: btnStyle, onClick: this.onRotateBtnClick.bind(this), name:'yR90'}, "yR+90"),
              e('button', {style: btnStyle, onClick: this.onRotateBtnClick.bind(this), name:'zR90'}, "zR+90"),
              e('br', null),
              e('br', null),
              e('div', {}, 
                e('fieldset', null, e('legend', {style: {textAlign: 'left'}}, "Glyphs On Cut:"),
                  e("label", null, e("input", {
                    type: "radio", value: "optOneSide", 
                    checked: this.state.selectedGCutOpt === 'optOneSide',
                    onChange: this.handleGlyphCutOptionChange.bind(this)
                  }), "One side"),
                  e("label", null, e("input", {
                    type: "radio", value: "optBothSide", 
                    checked: this.state.selectedGCutOpt === 'optBothSide',
                    onChange: this.handleGlyphCutOptionChange.bind(this)
                  }), "Both sides"),
                  e('br', null),
                  e('br', null),
                  e('label', null, "Distance scale factor"), 
                            e('input', {type:"number", style:{width:'50px', marginLeft:'10px', borderWidth:'1px', borderColor:'black'},
                                        value:this.state.cutHalfWidthScaleFactor, 
                                        onChange:e => this.handleDistanceChange(e)}),
                            e('button', {onClick:this.onApplyCutWidthScaleFactor.bind(this),
                                       style : { borderRadius: '4px', border: '2px solid #4CAF50', width: '50px', marginLeft:'10px'}}, "apply")
                           )),
              e('br', null),
              e('input', {type:"checkbox", onChange:this.onHideSurface.bind(this)}), e('label', null, "Hide surface"),
              e('br', null),
              e('br', null)
    );

  }
}

class dlgCuttingPlane extends React.Component {
  constructor(props) {
    super(props);
    this.state = { liked: false };
  }

  render() {
    const panelStyle = {
      position: 'absolute',
      minWidth: '300px', 
      zIndex: '9',
      textAlign: 'center',
      border: '1px solid #d3d3d3',
      backgroundColor:  '#f1f1f1'
    };

    var dlg = e("div",  {id : 'dlg_cuttingplane', style: panelStyle}, 
                        e(dlgHeader, {title: 'Cutting Plane'}), 
                        e(dlgCuttingPlaneContents));
    return dlg;
  }
}

function showCuttingPlaneDialog(aShow) {
  if (aShow)
    hideAllDialogs();

  var dlg = document.getElementById("dlg_cuttingplane");
  if (aShow && !dlg) {
    ReactDOM.render(e(dlgCuttingPlane), document.querySelector('#divCuttingPlane'));
    dragElement(document.getElementById("dlg_cuttingplane"));
  } else if (dlg) {
    dlg.style.display = aShow ? "block" : "none";
  }
}

//
//Open case dialog
//
class dlgOpenCaseContents extends React.Component {
  constructor(props) {
    super(props);
    this.state = {value:this.props.value};
  }

  onRadioBtnClick(ev) {
    const value = ev.target.value;
    const ps = this.props;
    const id = ev.target.id; //the index
    this.setState({value:value});

    var lStudies = editor.xhxDoc.studyList;
    if (id >= 0 && id < lStudies.length) {
      //find the selected study
      var lSelectedStudy = lStudies[id];

      //load the selected study
      if (lSelectedStudy !== undefined) {
        editor.xhxDoc.studyIdx = id;
        editor.xhxDoc.study = lSelectedStudy;//mark the active study.
        editor.loader.xhxLoadStudy(lSelectedStudy); //load the data into the selected study (only the data part is updated in the loading)
      }
    }
  }

  buildRadioButtons(arr) {
    return arr.map((choice, i) => {
      return  e("div", {
          style:{textAlign: 'left'},
          key:i //requied to avoid a warning
        }, 
        e("input", { 
          type: 'radio', 
          value: choice, 
          id: i,
          style:{marginLeft:'20px', marginTop: '6px', marginBottom: '6px'},
          checked: this.state.value == choice, 
          onChange: this.onRadioBtnClick.bind(this)}),  
        e("label", null, choice))
    })
  }

  render() {
    return e("div", null, this.buildRadioButtons(this.props.data))
  }
};

class dlgOpenCase extends React.Component {
  constructor(props) {
    super(props);
    this.state = {};
  }

  render() {
    const panelStyle = {
      position: 'absolute',
      minWidth: '300px', 
      zIndex: '9',
      textAlign: 'center',
      border: '1px solid #d3d3d3',
      backgroundColor:  '#f1f1f1'
    };

    var dlg = e("div",  {id : 'dlg_opencase', style: panelStyle}, 
                        e(dlgHeader, {title: 'Open Case'}), 
                        e(dlgOpenCaseContents, {data:this.props.data, value: this.props.data[0]}))
    return dlg;
  }
}

function showOpenCaseDialog(aShow, studyNames) {
  if (aShow)
    hideAllDialogs();
  var dlg = document.getElementById("dlg_opencase");
  if (aShow && !dlg) {
    var qChoices = 
      {  
        "questionType": "radio", 
        "questionText": "OpenFOAM Cases:", 
        "choices": [
        ]
      }
    

    for (var i = 0; i < studyNames.length; ++i) {
      qChoices.choices.push(studyNames[i]);
    }
  
    ReactDOM.render(e(dlgOpenCase, {data: qChoices.choices}), document.querySelector('#divOpenCase'));
    
    dragElement(document.getElementById("dlg_opencase"));
  } else if (dlg) {
    dlg.style.display = aShow ? "block" : "none";
  }
}

//
//Select result dialog
//
class dlgSelectRlt extends React.Component {
  constructor(props) {
    super(props);
    this.state = {mshs:[], rlts:[]};
    forceUpdateSelectRltDlg = this.cbForceUpdate.bind(this);
  }

  componentDidMount() {
    this.fetchData();
  }

  cbForceUpdate() {
    this.fetchData();
  }

  fetchData() {
    var _this = this;

    var study = editor.xhxDoc.study;
    var mshs = [];
    for ( var i = 0; i < study.data.parts.length; i++ ) {
      var rlt = "Part_" + i;	
      mshs.push(rlt);
    }
  
    var rlts = ['mesh']; //first is the mesh (display mesh)
    for ( var i = 0; i < study.data.results.length; i++ ) {
      var rlt = study.data.results[i];	
      rlts.push(rlt.name);
    }

    var rltIdx = 0;//mesh by default
    if (study.data.resultIdx != undefined && study.data.resultIdx >= 0)
      rltIdx = study.data.resultIdx + 1;

      _this.setState({mshs:mshs, rlts:rlts, selectedRlt:rltIdx});
  }

  onCheckBoxClick(ev) {
    const value = ev.target.value;
    const id = ev.target.id; //the index
    const checked = ev.target.checked;
    this.setState({value:value});

    editor.signals.xhxProjectSettingsChanged.dispatch(15, {visible:checked, partIdx:id});//15: part visibility
  }

  onRadioBtnClick(ev) {
    const id = ev.target.id; //the index
    this.setState({selectedRlt:id});

    var study = editor.xhxDoc.study;
    var resultIdx = id - 1;//-1: for mesh display
    var iIndpIdx = -1;

    editor.signals.xhxLoaded.dispatch(9, {'study':study, 'rltIdx':resultIdx, 'rltIndpIdx':iIndpIdx}); //9: load result
  }

  buildCheckBoxButtons() {
    var arr = this.state.mshs;
    return arr.map((choice, i) => {
      return  e("div", {
          style:{textAlign: 'left'},
          key:i //requied to avoid a warning
        }, 
        e("input", { 
          type: 'checkbox', 
          value: choice, 
          id: i,
          style:{marginLeft:'20px', marginTop: '6px', marginBottom: '6px'},
          defaultChecked: true, 
          onChange: this.onCheckBoxClick.bind(this)}),  
        e("label", null, choice))
    })
  }

  buildRadioButtons() {
    var arr = this.state.rlts;
    return arr.map((choice, i) => {
      return  e("div", {
          style:{textAlign: 'left'},
          key:i //requied to avoid a warning
        }, 
        e("input", { 
          type: 'radio', 
          value: choice, 
          id: i,
          style:{marginLeft:'20px', marginTop: '6px', marginBottom: '6px'},
          checked: this.state.selectedRlt == i, 
          onChange: this.onRadioBtnClick.bind(this)}),  
        e("label", null, choice))
    })
  }

  render() {
    const panelStyle = {
      position: 'absolute',
      minWidth: '260px', 
      zIndex: '9',
      textAlign: 'center',
      border: '1px solid #d3d3d3',
      backgroundColor:  '#f1f1f1'
    };

    var dlg = e("div",  {id : 'dlg_selectrlt', style: panelStyle}, 
                  e(dlgHeader, {title: 'Select Result'}), 
                  e("div", null, 
                    'Mesh Components', this.buildCheckBoxButtons(),
                    'Results', this.buildRadioButtons()
                  )
              )
    return dlg;
  }
}

var forceUpdateSelectRltDlg = undefined;
function showSelectRltDialog(aShow) {
  if (aShow)
    hideAllDialogs();
  var dlg = document.getElementById("dlg_selectrlt");
  if (aShow && !dlg) {
    
    ReactDOM.render(e(dlgSelectRlt, {}), document.querySelector('#divSelectRlt'));
    
    dragElement(document.getElementById("dlg_selectrlt"));
  } else if (dlg) {
    dlg.style.display = aShow ? "block" : "none";
    if (aShow && forceUpdateSelectRltDlg)
      forceUpdateSelectRltDlg();
  }
}

//
//display options dialog
//
class dlgDisplayOptions extends React.Component {
  constructor(props) {
    super(props);
    this.state = this.fetchData();
    forceUpdateDisplayOptionsDlg = this.cbForceUpdate.bind(this);
  }

  fetchData() {
    var selectedTime = 0.0;
    var timeValues = [];
    var nCmpts = 1; //number of components
    if (editor) {
      var study = editor.xhxDoc.study;
      if (study) {
        var resultIdx = study.data.resultIdx;
        if (resultIdx >= 0) {
          var rltInfo = study.data.results[resultIdx];
          nCmpts = rltInfo.nCmpts;
          var valArr = rltInfo.indpVals[0]; 
          var indpCount = valArr.length;
          for (var i = 0; i < indpCount; i++) {
            timeValues.push(valArr[i]);
          }

          var indpIdx = study.data.resultIndpIdx;
          if (indpIdx >= 0 && indpIdx < timeValues.length) 
            selectedTime = timeValues[indpIdx];
        }
      }
    }

    var ret = { 
      timeValArr : timeValues,
      selectedMeshDisplayOpt: 'optDisplayFeatureEdges',
      selectedVectorDisplayOpt: 'optDisplayVectorMagnitude',
      selectedTime: selectedTime,
      isoSurfaceEnabled: false,
      isoSurfaceCount: '5', //number of iso-surfaces
      glyphScaleFactor: '1.0',
      gridLineVisible: true,
      nCmpts: nCmpts
    };

    return ret;
  }

  cbForceUpdate() {
    this.setState(this.fetchData());
  }

  handleMeshDisplayOptionChange(event) {
    this.setState({
      selectedMeshDisplayOpt: event.target.value
    });

    var featureEdgeVisible = event.target.value == 'optDisplayFeatureEdges';
    var meshEdgeVisible = event.target.value == 'optDisplayMeshEdges';
    editor.signals.xhxProjectSettingsChanged.dispatch(6, featureEdgeVisible);//6: feature edges display
    editor.signals.xhxProjectSettingsChanged.dispatch(5, meshEdgeVisible);//5: mesh edges display
  }

  handleVectorDisplayOptionChange(event) {
    this.setState({
      selectedVectorDisplayOpt: event.target.value
    });

    var bEnableGlyphDisplay = true;
    if (event.target.value == 'optDisplayVectorMagnitude') {
      bEnableGlyphDisplay = false;
    }
    editor.signals.xhxProjectSettingsChanged.dispatch(8, bEnableGlyphDisplay);//8: glyph
  }

  handleGridLineVisibleChange(event) {
    this.setState({
      gridLineVisible: event.target.checked
    });

    editor.signals.xhxProjectSettingsChanged.dispatch(17, {visible:event.target.checked});//17: grid line display
  }

  handleTimeChange(event) {
    this.setState({
      selectedTime: event.target.value
    });

    //find the index
    var opts = event.target.children;
    var index = -1;
    for (var i = 0; i < opts.length; i++) {
      if (event.target.value == opts[i].value) {
        index = i;
        break;
      }
    }
    if (index >= 0) {
      var study = editor.xhxDoc.study;
      var resultIdx = study.data.resultIdx;
      editor.signals.xhxLoaded.dispatch(9, {'study':study, 'rltIdx':resultIdx, 'rltIndpIdx':index}); //9: load result

      var infoCombo = document.getElementById("infoCombo");
      if (infoCombo) {
        infoCombo.selectedIndex = index;
      }
    }
  }

  handleIsoSurfaceCheckBoxChange(event) {
    this.setState({
      isoSurfaceEnabled: event.target.checked
    });

    editor.signals.xhxProjectSettingsChanged.dispatch(0, event.target.checked);//0: enable/disable contours
  }

  handleIsoSurfaceCountChange(event) {
    this.setState({
      isoSurfaceCount: event.target.value
    });
  }

  onApplyNumberOfIsoSurfaces(e) {
    var n = this.state.isoSurfaceCount;
    editor.signals.xhxProjectSettingsChanged.dispatch(1, n ); //1: number of iso-surfaces		
  }

  handleGlyphScaleFactorChange(event) {
    this.setState({
      glyphScaleFactor: event.target.value
    });
  }

  onApplyGlyphScaleFactor(e) {
    var scaleFactor = this.state.glyphScaleFactor;
    editor.signals.xhxProjectSettingsChanged.dispatch(9, scaleFactor); //9: scale factor of glyphs		
  }

  render() {
    const panelStyle = {
      position: 'absolute',
      minWidth: '300px', 
      zIndex: '9',
      textAlign: 'center',
      border: '1px solid #d3d3d3',
      backgroundColor:  '#f1f1f1'
    };

    var elements = [];

    var eleTimeSteps = e('fieldset', null, e('legend', {style: {textAlign: 'left'}}, "Time Step:"),
        e('label', null, "Time value: "),
        e('select', {value: this.state.selectedTime, onChange: this.handleTimeChange.bind(this)}, 
              this.state.timeValArr.map((word, idx) => {
                return e("option", {key: idx}, word);
              })
        )
      );
    elements.push(eleTimeSteps);

    var eleMeshDisplay =  e('fieldset', null, e('legend', {style: {textAlign: 'left'}}, "Mesh Display:"),
        e("label", null, e("input", {
          type: "radio", value: "optDisplayFeatureEdges", 
          checked: this.state.selectedMeshDisplayOpt === 'optDisplayFeatureEdges',
          onChange: this.handleMeshDisplayOptionChange.bind(this)
        }), "Feature edges"),
        e("label", null, e("input", {
          type: "radio", value: "optDisplayMeshEdges", 
          checked: this.state.selectedMeshDisplayOpt === 'optDisplayMeshEdges',
          onChange: this.handleMeshDisplayOptionChange.bind(this)
        }), "Mesh edges"),
        e("label", null, e("input", {
          type: "radio", value: "optDisplayNoEdges", 
          checked: this.state.selectedMeshDisplayOpt === 'optDisplayNoEdges',
          onChange: this.handleMeshDisplayOptionChange.bind(this)
        }), "No edges")
      );
    elements.push(eleMeshDisplay);

    var eleIsoSurfaces = e('fieldset', null, e('legend', {style: {textAlign: 'left'}}, "Iso-Surfaces:"),
        e('input', {type:'checkbox', checked: this.state.isoSurfaceEnabled, onChange: this.handleIsoSurfaceCheckBoxChange.bind(this)}), 
        e('label', null, 'Enable'),
        e('br', null),
        e('label', null, "Number of iso-surfaces: "), 
        e('input', {type:"number", step:'1', pattern:"\d+", style:{width:'50px', marginLeft:'10px', borderWidth:'1px', borderColor:'black'},
                              value:this.state.isoSurfaceCount, 
                              onChange:e => this.handleIsoSurfaceCountChange(e)}),
        e('button', {onClick:this.onApplyNumberOfIsoSurfaces.bind(this),
                  style : { borderRadius: '4px', border: '2px solid #4CAF50', width: '50px', marginLeft:'10px'}}, "apply")
      );
    elements.push(eleIsoSurfaces);

    if (this.state.nCmpts == 3) {
        var eleVectorDisplay = e('fieldset', {style: {textAlign: 'left'}}, e('legend', {style: {textAlign: 'left'}}, "Vector Display:"),
            e("label", null, e("input", {
              type: "radio", value: "optDisplayVectorMagnitude", 
              checked: this.state.selectedVectorDisplayOpt === 'optDisplayVectorMagnitude',
              onChange: this.handleVectorDisplayOptionChange.bind(this)
            }), "Component or magnitude"),
            e('br', null),
            e("label", null, e("input", {
              type: "radio", value: "optDisplayVectorGlyphs", 
              checked: this.state.selectedVectorDisplayOpt === 'optDisplayVectorGlyphs',
              onChange: this.handleVectorDisplayOptionChange.bind(this)
            }), "Glyphs"),
            e('br', null),
            e('label', null, "Glyph scale factor: "), 
            e('input', {type:"number", style:{width:'50px', marginLeft:'10px', borderWidth:'1px', borderColor:'black'},
                                  value:this.state.glyphScaleFactor, 
                                  onChange:e => this.handleGlyphScaleFactorChange(e)}),
            e('button', {onClick:this.onApplyGlyphScaleFactor.bind(this),
              style : { borderRadius: '4px', border: '2px solid #4CAF50', width: '50px', marginLeft:'10px'}}, "apply")
        );
        elements.push(eleVectorDisplay);
    }

    var eleGridLine =  e('fieldset', {style: {textAlign: 'left'}}, e('legend', {style: {textAlign: 'left'}}, "Grid Lines:"),
        e('input', {type:'checkbox', checked: this.state.gridLineVisible, onChange: this.handleGridLineVisibleChange.bind(this)}), 
        e('label', null, 'Visible')
      );
    elements.push(eleGridLine)

    var dlg = e("div",  {id : 'dlg_displayoptions', style: panelStyle}, 
                e(dlgHeader, {title: 'Display Options'}), 
                elements //the contents
              );
    return dlg;
  }
}

//work around, there may be better way
var forceUpdateDisplayOptionsDlg = undefined;
function showDisplayOptionsDialog(aShow) {
  if (aShow)
    hideAllDialogs();
  var dlg = document.getElementById("dlg_displayoptions");
  if (aShow && !dlg) {
    ReactDOM.render(e(dlgDisplayOptions), document.querySelector('#divDisplayOptions'));
    dragElement(document.getElementById("dlg_displayoptions"));
  } else if (dlg) {
    dlg.style.display = aShow ? "block" : "none";
    if (forceUpdateDisplayOptionsDlg)
      forceUpdateDisplayOptionsDlg();
  }
}

//copied from W3C
function dragElement(elmnt) {
  var pos1 = 0, pos2 = 0, pos3 = 0, pos4 = 0;

  //assume the first child is the header
  elmnt.firstChild.onmousedown = dragMouseDown;

  function dragMouseDown(e) {
    e = e || window.event;
    e.preventDefault();
    // get the mouse cursor position at startup:
    pos3 = e.clientX;
    pos4 = e.clientY;
    document.onmouseup = closeDragElement;
    // call a function whenever the cursor moves:
    document.onmousemove = elementDrag;
  }

  function elementDrag(e) {
    e = e || window.event;
    e.preventDefault();
    // calculate the new cursor position:
    pos1 = pos3 - e.clientX;
    pos2 = pos4 - e.clientY;
    pos3 = e.clientX;
    pos4 = e.clientY;
    // set the element's new position:
    elmnt.style.top = (elmnt.offsetTop - pos2) + "px";
    elmnt.style.left = (elmnt.offsetLeft - pos1) + "px";
  }

  function closeDragElement() {
    /* stop moving when mouse button is released:*/
    document.onmouseup = null;
    document.onmousemove = null;
  }
}

//buton bar

class dlgButtonBar extends React.Component {
  constructor(props) {
    super(props);
    this.state = { liked: false };
  }

  onRotateBtnClick(event) {
    if (event.target.name == 'Open') {
      if (IsInMode("openfoam")) {
        editor.loader.xhxLoadStudyList(); 
      } else {
        fileInput.click();
      }
    } else
    if (event.target.name == 'Results') {
      editor.signals.xhxProjectSettingsChanged.dispatch(10, 'selectRlt');//10: open the named dialog
    } else
    if (event.target.name == 'Cutting') {
      showCuttingPlaneDialog(true);
    } else
    if (event.target.name == 'Options') {
      showDisplayOptionsDialog(true);
    }
  }
  render() {
    const panelStyle = {
      position: 'absolute',
      minWidth: '410px', 
      zIndex: '9',
      textAlign: 'center',
      top:2,
      border: '1px solid #d3d3d3',
      backgroundColor:  '#f1f1f1'
    };
    const btnStyle = {
      borderRadius: '4px', 
      border: '2px solid #4CAF50', 
      width: '25%'
    };

    var dlg = e("div",  {id : 'dlg_buttonbar', style: panelStyle}, 
      e('button', {style: btnStyle, onClick: this.onRotateBtnClick.bind(this), name:'Open'}, "Open"),
      e('button', {style: btnStyle, onClick: this.onRotateBtnClick.bind(this), name:'Results'}, "Results"),
      e('button', {style: btnStyle, onClick: this.onRotateBtnClick.bind(this), name:'Cutting'}, "Cutting Plane"),
      e('button', {style: btnStyle, onClick: this.onRotateBtnClick.bind(this), name:'Options'}, "Options")
    );
    return dlg;
  }
}

function showButtonBar() {
  var dlg = document.getElementById("dlg_buttonbar");
  if (!dlg) {
    ReactDOM.render(e(dlgButtonBar), document.querySelector('#divMenuButtons'));
  } else {
    dlg.style.display = "block";
  }
}

function hideAllDialogs() {
  showCuttingPlaneDialog(false);
  showDisplayOptionsDialog(false);
  showOpenCaseDialog(false);
  showSelectRltDialog(false);
}
