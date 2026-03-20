(function(){
  var key = "espeed32-doc-theme";
  var root = document.documentElement;
  var select = document.getElementById("theme-select");
  if (!select) return;

  var saved = localStorage.getItem(key);
  if (saved !== "light" && saved !== "dark" && saved !== "system") saved = "system";
  select.value = saved;

  function apply(mode){
    if (mode === "light" || mode === "dark") root.setAttribute("data-theme", mode);
    else root.removeAttribute("data-theme");
  }

  apply(saved);
  select.addEventListener("change", function(){
    localStorage.setItem(key, select.value);
    apply(select.value);
  });

  var path = (window.location.pathname || "").toLowerCase();
  var activeLang = (root.getAttribute("lang") || "en").toLowerCase();
  if (path.indexOf("/docs/no") !== -1) activeLang = "no";
  else if (path.indexOf("/docs/en") !== -1) activeLang = "en";
  else if (path.indexOf("/docs/es") !== -1) activeLang = "es";
  else if (path.indexOf("/docs/de") !== -1) activeLang = "de";
  else if (path.indexOf("/docs/it") !== -1) activeLang = "it";

  var anchorLabels = {
    en: "Link to this heading",
    no: "Lenke til denne overskriften",
    es: "Enlace a este titulo",
    de: "Link zu dieser Uberschrift",
    it: "Link a questo titolo"
  };

  function slugify(text){
    var slug = String(text || "").toLowerCase();
    slug = slug
      .replace(/[æ]/g, "ae")
      .replace(/[ø]/g, "o")
      .replace(/[å]/g, "a")
      .replace(/[ä]/g, "a")
      .replace(/[ö]/g, "o")
      .replace(/[ü]/g, "u")
      .replace(/[ß]/g, "ss")
      .replace(/[ñ]/g, "n");
    if (slug.normalize) slug = slug.normalize("NFKD").replace(/[\u0300-\u036f]/g, "");
    slug = slug.replace(/^\d+[\.\)]\s*/, "");
    slug = slug.replace(/[^a-z0-9]+/g, "-").replace(/^-+|-+$/g, "");
    return slug || "section";
  }

  function collectUsedIds(){
    var used = {};
    var nodes = document.querySelectorAll("[id]");
    for (var i = 0; i < nodes.length; i++) {
      var id = nodes[i].id;
      if (!id) continue;
      used[id] = (used[id] || 0) + 1;
    }
    return used;
  }

  function makeUniqueId(base, used){
    if (!used[base]) {
      used[base] = 1;
      return base;
    }
    var index = 2;
    var next = base + "-" + index;
    while (used[next]) {
      index += 1;
      next = base + "-" + index;
    }
    used[next] = 1;
    return next;
  }

  function addHeadingAnchors(){
    var used = collectUsedIds();
    var label = anchorLabels[activeLang] || anchorLabels.en;
    var targets = document.querySelectorAll("h2, h3, h4, details.doc-fold > summary");
    for (var i = 0; i < targets.length; i++) {
      var target = targets[i];
      target.classList.add("anchor-target");
      if (!target.id) target.id = makeUniqueId(slugify(target.textContent), used);
      if (target.querySelector(".heading-anchor")) continue;
      var link = document.createElement("a");
      link.className = "heading-anchor";
      link.href = "#" + target.id;
      link.setAttribute("aria-label", label);
      link.setAttribute("title", label);
      link.addEventListener("click", function(event){
        event.stopPropagation();
      });
      target.appendChild(link);
    }
  }

  function openAncestorDetails(node){
    var current = node;
    while (current) {
      if (current.tagName === "DETAILS") current.open = true;
      current = current.parentElement;
    }
  }

  function getHashTarget(){
    var hash = window.location.hash || "";
    if (!hash || hash.charAt(0) !== "#") return null;
    var id = hash.slice(1);
    try {
      id = decodeURIComponent(id);
    } catch (err) {}
    return document.getElementById(id);
  }

  function revealHashTarget(){
    var target = getHashTarget();
    if (!target) return;
    openAncestorDetails(target);
    target.scrollIntoView();
  }

  addHeadingAnchors();
  revealHashTarget();
  window.addEventListener("hashchange", revealHashTarget);

  var langLinks = document.querySelectorAll("a[data-doc-lang]");
  for (var i = 0; i < langLinks.length; i++) {
    var link = langLinks[i];
    if (link.getAttribute("data-doc-lang") === activeLang) link.classList.add("lang-active");
    else link.classList.remove("lang-active");
  }
})();